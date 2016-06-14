#include "SteamAppInfo.h"

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <sstream>

#include <iostream>

#ifdef _MSC_VER
#define PRELIT(x) L##x
#else
#define PRELIT(x) x
#endif

SteamAppInfo::SteamAppInfo(const std::string steamCMDBin) : 
    m_steamBin{steamCMDBin},
    m_appInfoPath{ boost::filesystem::path(steamCMDBin).parent_path().string() + "/appcache/appinfo.vdf" }, //todo: check it on linux
    m_lastModified{ 0 } 
{
    updateCache();
}


SteamAppInfo::~SteamAppInfo() {}

bool SteamAppInfo::updateCache() const
{

    std::string steamCMDArgs = " +login anonymous +app_info_update 1 +app_info_print +quit"; //appinfo + print should make sure the file is created
    auto c = boost::process::execute(
        boost::process::initializers::run_exe(m_steamBin),
        boost::process::initializers::set_cmd_line(steamCMDArgs),
        boost::process::initializers::start_in_dir(boost::filesystem::path(m_steamBin).parent_path()),
        boost::process::initializers::close_stdout()
    );
    boost::process::wait_for_exit(c);

    if (!boost::filesystem::exists(m_appInfoPath))
        throw std::exception("appInfoPath does not exist");


    auto lastModified = boost::filesystem::last_write_time(m_appInfoPath);
    if (lastModified <= m_lastModified)
        return false;

    m_appIdPosMap.clear();

    m_lastModified = lastModified;
    std::cout << "Read cache file...";
    std::basic_ifstream<unichar_t> file(m_appInfoPath, std::ios::binary);

    file.seekg(0, std::ios::end);
    m_cacheString.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&m_cacheString[0], m_cacheString.size());
    file.close();
    std::cout << " Done\n";
    return true;
}

size_t SteamAppInfo::searchCacheForNumber(unsigned number, size_t pos) const
{
    if (number == 0)
        return std::string::npos;

    unichar_t appIdString[sizeof(number) + 3];
    std::copy(reinterpret_cast<char*>(&number), reinterpret_cast<char*>(&number) + sizeof(number), appIdString + 1);
    appIdString[sizeof(number) + 2] = '\0';
    appIdString[0] = appIdString[sizeof(number) + 1] = spaceChar;

    size_t p = m_cacheString.find(appIdString + 1);

    return p;
}

size_t SteamAppInfo::searchCacheForString(std::string str, size_t pos) const
{
    return m_cacheString.find(uniString(str.cbegin(), str.cend()) + spaceChar, pos);
}

size_t SteamAppInfo::getAppPos(const std::string& appId) const
{
    if (m_appIdPosMap.find(appId) == m_appIdPosMap.end())
        m_appIdPosMap[appId] = searchCacheForNumber(std::atoi(appId.c_str()));
    return m_appIdPosMap[appId];
}

std::string SteamAppInfo::getValue(std::string key, const std::string& appId) const
{
    // format: key and value are seperated by 0x20 and alternating
    // in front of key, the value type is specified with a number
    // 01 -> string
    // 02 -> number (unsigned?)
    // TODO: 
    // - use real uft8 encoding via boost
    // - you should use variants for the return value
    size_t appPos = getAppPos(appId);
    appPos = searchCacheForString(key, appPos);
    appPos += key.size()+1;
    std::string result;
    if (appPos == uniString::npos)
        return "";
    while (m_cacheString[appPos] != spaceChar)
        result.push_back(m_cacheString[appPos++]);
    return result;
}

std::string SteamAppInfo::getBuildId(const std::string& appId, const std::string& branch) const
{
    size_t pos = getAppPos(appId);

    if (pos == uniString::npos)
        return "";

    pos = m_cacheString.find(PRELIT("branches"), pos);
    auto appEndPos = m_cacheString.find(PRELIT("common"), pos);

    pos = m_cacheString.find(uniString{branch.begin(), branch.end()}, pos);

    if (pos >= appEndPos)
        return "";

    auto pos2 = pos;
    while (m_cacheString[pos2] != 0)
        std::cout << (char)m_cacheString[pos2++];
    std::cout << std::endl;    

    pos = m_cacheString.find(PRELIT("buildid"), pos);

    //read build id in reverse order
    pos += sizeof("buildid");
    size_t endPos = pos - 1;
    pos = m_cacheString.find(spaceChar, pos) - 1;
    std::stringstream cacheBuildId;
    cacheBuildId << std::hex;
    while (endPos != pos)
        cacheBuildId << m_cacheString[pos--];

    //convert from hex to dec
    long long appIdNum;
    cacheBuildId >> appIdNum;

    return std::to_string(appIdNum);
}