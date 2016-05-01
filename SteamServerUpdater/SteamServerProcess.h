#pragma once

#include <string>
#include <vector>
#include <future>

#include <boost/process.hpp>
#include <boost/process/mitigate.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

#include "SteamAppInfo.h"

class SteamServerProcess
{
    const std::string m_appID;
    const std::string m_exePath;
    const std::string m_Args;
    const std::string m_manifestPath;
    boost::process::child* m_process;
    std::future<void> m_runThread;
    struct ManifestData
    {
        std::string path;
        std::string name;
        std::string installedBuildId;
    } m_manifest;
    /// get the version from the current installation
    ManifestData getManifest(const std::string& steamManifestDir);
public:
    struct Init
    {
        std::string appId;
        std::string args;
        std::string installDir;
        std::string manifestPath;
    };

    SteamServerProcess(const SteamAppInfo &appInfo, const Init& i)
        :m_appID(i.appId),
        m_exePath(i.installDir+"\\"+appInfo.getValue("executable",i.appId)), 
        m_Args(i.args),
        m_manifestPath(i.manifestPath),
        m_process(nullptr), 
        m_runThread(), 
        m_manifest(getManifest(i.manifestPath))
    {}
    ~SteamServerProcess() 
    {
        terminateProgram();
    }

    std::string getAppName()const { return m_manifest.name; }

    std::string getAppId()const { return m_appID; }

    std::string getInstalledBuildId() const
    {
        return m_manifest.installedBuildId;
    }

    void startProgram()
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


        //startFunc();
        //todo: detect if server crashed, if so, restart
        const bool restartOnCrash = true;
        m_runThread = std::async(std::launch::async, [this, restartOnCrash](){
            
            int exit = -1;         
            //do {
               
                //wait until terminate
                exit = BOOST_PROCESS_EXITSTATUS(boost::process::wait_for_exit(*m_process));
                delete m_process;
                m_process = nullptr;
            //} while (exit && restartOnCrash);
        });
    }

    bool isRunning()const
    {
        return m_process != nullptr;
    }

    /// run the program and wait for exit
    void waitFor(std::size_t ms)const
    {
        m_runThread.wait_for(std::chrono::milliseconds(ms));
    }

    void stopProgram()
    {
        terminateProgram();
    }

    /// forces the program to terminate
    void terminateProgram()
    {
        if (!m_process)
            return;
        boost::process::terminate(*m_process);
        delete m_process;
        m_process = nullptr;
    }
};