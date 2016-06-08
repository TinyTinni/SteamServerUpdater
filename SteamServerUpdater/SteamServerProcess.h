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
    const std::string m_branch;
    const std::string m_manifestPath;

    boost::process::child* m_process;
    std::future<void> m_processThread;
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
        std::string branch;
    };

    SteamServerProcess(const SteamAppInfo &appInfo, const Init& i)
        :m_appID(i.appId),
        m_exePath(i.installDir+"\\"+appInfo.getValue("executable",i.appId)), 
        m_Args(i.args),
        m_branch(i.branch),
        m_manifestPath(i.manifestPath),
        m_process(nullptr), 
        m_processThread(),
        m_manifest(getManifest(i.manifestPath))
    {}
    ~SteamServerProcess() 
    {
        terminateProgram();
    }

    std::string getAppName()const { return m_manifest.name; }

    std::string getBranch()const { return m_branch;  }

    std::string getAppId()const { return m_appID; }

    std::string getInstalledBuildId() const
    {
        return m_manifest.installedBuildId;
    }

    void startProgram();

    bool isRunning()const
    {
        return m_process != nullptr;
    }

    /// run the program and wait for exit
    void waitFor(std::size_t ms)const
    {
        m_processThread.wait_for(std::chrono::milliseconds(ms));
    }

    /// stops the program. TODO: reimplement with custom stop function per application
    void stopProgram()
    {
        terminateProgram();
    }

    /// forces the program to terminate
    void terminateProgram();
};