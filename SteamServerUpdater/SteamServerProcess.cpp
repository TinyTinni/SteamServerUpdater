#include "SteamServerProcess.h"

#include <fstream>
#include <algorithm>
#include <stdexcept>

#include "SteamAppInfo.h"




std::string stripQuotes(const std::string& inOut)
{
    std::string result(inOut.begin() + 1, inOut.end() - 1);
    return result;
}

SteamServerProcess::ManifestData SteamServerProcess::getManifest(const std::string& steamManifestDir)
{
    ManifestData result;
    result.path = steamManifestDir;

    //extract version string
    try
    {
        std::ifstream file(steamManifestDir);
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
        throw std::length_error(std::string("Could not find ") + e.what() + " in the manifest file. Manifest path: " + steamManifestDir);
    }

    return result;
}

void SteamServerProcess::terminateProgram()
{
    if (!m_process)
        return;
    boost::process::terminate(*m_process);
    m_processThread.wait();
    // delete and set ptr to null is done by process thread
}

void SteamServerProcess::startProgram()
{
    if (m_process)
        return; //already running

                //todo: reread manifest only if necessary (remeber last modified)
    m_manifest = getManifest(m_manifestPath);

    std::cout << "Start Server: " << m_exePath << std::endl;
    //auto startFunc = [this]()
    //{
    using namespace boost::process::initializers;
    m_process = new boost::process::child(boost::process::execute(
        run_exe(m_exePath),
        set_cmd_line(m_Args),
        start_in_dir(boost::filesystem::path(m_exePath).parent_path())
    ));
    //};

    Sleep(5000);


    //startFunc();
    //todo: detect if server crashed, if so, restart
    const bool restartOnCrash = true;
    m_processThread = std::async(std::launch::async, [this, restartOnCrash]() {

        int exit = -1;
        //do {

        //wait until terminate
        exit = BOOST_PROCESS_EXITSTATUS(boost::process::wait_for_exit(*m_process));
        delete m_process;
        m_process = nullptr;
        //} while (exit && restartOnCrash);
    });
    m_processThread.wait_for(std::chrono::milliseconds(0));
}
