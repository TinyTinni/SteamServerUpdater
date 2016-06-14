#include "SteamServerProcess.h"

#include <fstream>
#include <algorithm>

#include <boost/filesystem.hpp>

#include <boost/process/mitigate.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

#include "SteamAppInfo.h"

std::string stripQuotes(const std::string& inOut)
{
    std::string result(inOut.begin() + 1, inOut.end() - 1);
    return result;
}

SteamServerProcess::ManifestData SteamServerProcess::getManifest(const std::string& steamManifestPath)
{
    if (!boost::filesystem::exists(steamManifestPath))
        throw file_find_exception(steamManifestPath, "Manifest file for app: " + getAppId());

    std::time_t lastModified = boost::filesystem::last_write_time(steamManifestPath);

    if (m_manifest.lastModified >= lastModified)
        return m_manifest;

    ManifestData result;

    result.lastModified = lastModified;
    result.path = steamManifestPath;

    //extract version string
    try
    {
        std::ifstream file(steamManifestPath);
        auto iter = std::find(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), "\"name\"");
        if (++iter == std::istream_iterator<std::string>())
            throw std::runtime_error("\"name\"");
        result.name = stripQuotes(*iter);
        while (result.name.back() != '\"' && iter != std::istream_iterator<std::string>())
            result.name += " " + *++iter;
        result.name = stripQuotes(result.name);

        iter = std::find(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), "\"buildid\"");
        if (++iter == std::istream_iterator<std::string>())
            throw std::runtime_error("\"buildid\"");

        //version string found, remove leading and ending quotes
        result.installedBuildId = stripQuotes(*iter);
    }
    catch (std::length_error &e)
    {
        throw std::length_error(std::string("Could not find ") + e.what() + " in the manifest file. Manifest path: " + steamManifestPath);
    }

    return result;
}

void SteamServerProcess::steamCMDUpdate()
{
    std::cout << "update Server" << std::endl;
    std::cout << "execute: " << m_steamCMD << m_steamCMDArgs << std::endl;
    auto c = boost::process::execute(
        boost::process::initializers::run_exe(m_steamCMD),
        boost::process::initializers::set_cmd_line(m_steamCMD + m_steamCMDArgs),
        boost::process::initializers::start_in_dir(boost::filesystem::path(m_steamCMD).parent_path())
    );
    boost::process::wait_for_exit(c);
    m_appInfo.updateCache();
}

void SteamServerProcess::createProcess()
{
    if (isRunning())
        return;

    //start server process
    //todo: reread manifest only if necessary (remeber last modified)
    m_manifest = getManifest(m_manifestPath);

    std::cout << "Start Server: " << m_exePath << " " << m_Args << std::endl;
    using namespace boost::process::initializers;
    m_process = std::make_unique<boost::process::child>(boost::process::execute(
        run_exe(m_exePath),
        set_cmd_line(m_exePath + " " + m_Args),
        start_in_dir(boost::filesystem::path(m_exePath).parent_path())
    ));

    //todo: detect if server crashed, if so, restart
    m_processThread = std::async(std::launch::async, [this]() ->int {

        int exit = -1;
        //wait until terminate
        exit = BOOST_PROCESS_EXITSTATUS(boost::process::wait_for_exit(*(this->m_process)));
        return exit;
    });
    m_processThread.wait_for(std::chrono::milliseconds(0));
}

void SteamServerProcess::checkUpdateLoop(size_t intervalInMS)
{
    size_t i = 0;
    while (isRunning())
    {
        waitFor(1000 * intervalInMS);
        // get steam cache update
        m_appInfo.updateCache();
        auto appInfoBuildId = m_appInfo.getBuildId(getAppId(), getBranch());
        // update app if neccessary
        if (getInstalledBuildId() != appInfoBuildId)
        {
            std::cout << "update from: " << getInstalledBuildId() << " to: " << appInfoBuildId << std::endl;
            stop();
            steamCMDUpdate();
            createProcess();
            ++i;
        }
    }
}

SteamServerProcess::SteamServerProcess(const SteamAppInfo & appInfo, Init i)
    :m_appID(i.appId),
    m_exePath(),
    m_Args(i.args),
    m_branch(i.branch),
    m_manifestPath(i.manifestPath),
    m_appInfo(appInfo),
    m_steamCMD(i.steamCMD),
    m_steamCMDArgs(),
    m_process(nullptr),
    m_processThread(),
    m_manifest()
{
    //get manifest and install dir
    if(i.installDir.empty())
    {
        const std::string steamCMDRoot = boost::filesystem::path(i.steamCMD).parent_path().string();
        i.installDir = steamCMDRoot + "/steamapps/" + appInfo.getValue("installDir", i.appId);
        m_manifestPath = steamCMDRoot + "/steamapps/steamapps";
    }
    else
    {
        m_manifestPath = i.installDir + "/steamapps";  
    }
    m_manifestPath += "/appmanifest_" + i.appId + ".acf";

    //check if installDir exists. ManifestPath will be checked later, everytime when its is read
    if (!i.installDir.empty() && !boost::filesystem::is_directory(i.installDir))
        throw file_find_exception(i.installDir, "InstallDir for app: " + getAppId());

    m_exePath = i.installDir + "\\" + appInfo.getValue("executable", i.appId);

    if (!boost::filesystem::exists(m_exePath))
        throw file_find_exception(m_exePath, "executable for app: "+ getAppId() +". Correct InstallDir?");

    // create steamCMD parameter string
    m_steamCMDArgs = " +login anonymous";
    if (!i.installDir.empty())
        m_steamCMDArgs += " +force_install_dir " + i.installDir;
    m_steamCMDArgs += " +app_update " + i.appId;
    if (!i.branch.empty() && i.branch != "public")
        m_steamCMDArgs += " -beta " + i.branch;
    if (!i.branchPW.empty())
        m_steamCMDArgs += " -betapassword " + i.branchPW;
    m_steamCMDArgs += " +quit";
}

void SteamServerProcess::terminateProgram()
{
    if (!isRunning())
        return;
    boost::process::terminate(*m_process);
    m_processThread.wait();
    m_process.reset();
    // delete and set ptr to null is done by process thread
}

void SteamServerProcess::run(size_t updateInMS, bool precheck)
{
    if (precheck)
        steamCMDUpdate();
    createProcess();
    checkUpdateLoop(updateInMS);
}
