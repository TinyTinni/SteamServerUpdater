#pragma once

#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include <ctime>

#include <boost/process.hpp>
#include <boost/exception/all.hpp>
#include "SteamAppInfo.h"

class SteamServerProcess
{
    const std::string m_appID;
    std::string m_exePath;
    const std::string m_Args;
    const std::string m_branch;
    std::string m_manifestPath;

    const SteamAppInfo& m_appInfo;

    const std::string m_steamCMD;
    std::string m_steamCMDArgs;

    std::unique_ptr<boost::process::child> m_process;
    std::future<int> m_processThread;
    struct ManifestData
    {
        std::string path;
        std::string name;
        std::string installedBuildId;
        std::time_t lastModified;
    } m_manifest;
    /// get the version from the current installation
    ManifestData getManifest(const std::string& steamManifestDir);

    void steamCMDUpdate();
    void createProcess();
    void checkUpdateLoop(size_t intervalInMS);
public:
    struct Init
    {
        std::string steamCMD;
        std::string appId;
        std::string args;
        std::string installDir;
        std::string manifestPath;
        std::string branch;
        std::string branchPW;
    };

    struct file_find_exception : virtual boost::exception
    {
        typedef boost::error_info<struct tag_file_find_exception_func_desc, std::string > func_desc;
        file_find_exception(const std::string& filename, const std::string& function = "") :
            boost::exception()
        {
            *this << func_desc(function) << boost::errinfo_file_name(filename);
        }
    };

    SteamServerProcess(const SteamAppInfo &appInfo, Init i);
    inline ~SteamServerProcess() 
    {
        terminateProgram();
    }

    inline std::string getAppName()const { return m_manifest.name; }

    inline std::string getBranch()const { return m_branch;  }

    inline std::string getAppId()const { return m_appID; }

    inline std::string getInstalledBuildId() const
    {
        return m_manifest.installedBuildId;
    }

    /** Creates and starts the process
    @param intervalInMS Give Interval between update checks in miliseconds
    @ param precheck If true, given app will be updated or installed (if not) before trying to start it
    */
    void run(std::size_t updateInMS, bool precheck = true);

    inline bool isRunning()const
    {
        return m_process != nullptr;
    }

    /// run the program and wait for exit
    inline void waitFor(std::size_t ms)const
    {
        m_processThread.wait_for(std::chrono::milliseconds(ms));
    }

    /// stops the program. TODO: reimplement with custom stop function per application
    inline void stop()
    {
        terminateProgram();
    }

    /// forces the program to terminate
    void terminateProgram();
};