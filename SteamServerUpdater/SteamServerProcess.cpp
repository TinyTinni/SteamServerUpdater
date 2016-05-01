#include "SteamServerProcess.h"

#include <fstream>
#include <algorithm>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "SteamAppInfo.h"



std::string stripQuotes(const std::string& inOut)
{
    std::string result(inOut.begin() + 1, inOut.end() - 1);
    return result;
}

SteamServerProcess::ManifestData SteamServerProcess::getManifest(const std::string& steamManifestDir)
{
    fs::path filepath = steamManifestDir + "/appmanifest_" + m_appID + ".acf";
    //if (!fs::exists(filepath))
    //    return "dont exists";

    ManifestData result;
    result.path = steamManifestDir;

    //extract version string
    std::ifstream file(filepath.string());
    auto iter = std::find(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), "\"name\"");
    //if (++iter == std::istream_iterator<std::string>())
    //    return "could not found name";


    result.name = *++iter;
    while (result.name.back() != '\"' && iter != std::istream_iterator<std::string>())
        result.name += " " + *++iter;
    result.name = stripQuotes(result.name);

    iter = std::find(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), "\"buildid\"");
    //if (++iter == std::istream_iterator<std::string>())
    //return "could not found buildid";

    //version string found, remove leading and ending quotes
    result.installedBuildId = stripQuotes(*++iter);

    return result;
}
