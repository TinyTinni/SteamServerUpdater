#include <iostream>
#include <functional>

#include "SteamServerProcess.h"
#include <boost/process.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/filesystem.hpp>

#include <boost/exception/all.hpp>

// Could not find file or directory. Holds additional information in errinfo_api_function
struct file_find_exception : virtual boost::exception 
{
    typedef boost::error_info<struct tag_file_find_exception_func_desc, std::string > func_desc;
    file_find_exception(const std::string& filename, const std::string& function = ""): 
        boost::exception()
    {
        *this << func_desc(function) << boost::errinfo_file_name(filename);
    }
}; 

struct IniStruct
{
    std::string appId;
    std::string installDir;
    std::string args;
    std::string branch;

    std::string steamCMD;
    size_t updateInterval;
};
 IniStruct readIniFile(const std::string& file)
{
    try
    {
        IniStruct initStruct;
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(file, pt);
        initStruct.appId = pt.get<std::string>("AppInfo.AppId");
        initStruct.args = pt.get<std::string>("AppInfo.Args");
        initStruct.branch = pt.get<std::string>("AppInfo.Branch");

        initStruct.installDir = pt.get<std::string>("AppInfo.InstallDir");
        if (!initStruct.installDir.empty() && !boost::filesystem::is_directory(initStruct.installDir))
            throw file_find_exception(initStruct.installDir, "InstallDir in .ini file");

        initStruct.steamCMD = pt.get<std::string>("General.SteamCMDBin");
        if (!boost::filesystem::exists(initStruct.steamCMD) || boost::filesystem::is_directory(initStruct.steamCMD))
            throw file_find_exception(initStruct.steamCMD, "Steam Binary in .ini file: ");

        initStruct.updateInterval = pt.get<size_t>("General.UpdateInterval");

        return initStruct;
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error(std::string("Mailformatted .ini file: ") + e.what());
    }
}

void runServer(const SteamAppInfo& appInfo, SteamServerProcess &p, size_t updateInSec, std::function<void()> updateFunc)
{
    p.startProgram();
    size_t i = 0;
    while (p.isRunning())
    {
        p.waitFor(1000 * updateInSec);
        // get steam cache update
        appInfo.updateCache();
        auto appInfoBuildId = appInfo.getBuildId(p.getAppId(), p.getBranch());
        // update app if neccessary
        if (p.getInstalledBuildId() != appInfoBuildId)
        {
            std::cout << "update from: " << p.getInstalledBuildId() << " to: " << appInfoBuildId << std::endl;
            p.stopProgram();
            updateFunc();
            p.startProgram();
            ++i;
        }
     }
    std::cout << "#Updates: " << i << std::endl;
}

int main(int argc, char** argv)
{
    try
    {        
        IniStruct ini = readIniFile("ServerInfo.ini");

        // Read cache file
        SteamAppInfo appInfo(ini.steamCMD);

        SteamServerProcess::Init processInit;
        processInit.appId = ini.appId;
        processInit.args = ini.args;
        processInit.branch = ini.branch.empty()? "public" : ini.branch;
        

        // extract manifest path for given appId and define installDir
        processInit.installDir = ini.installDir;
        if (ini.installDir.empty())
        {
            const std::string steamCMDRoot = boost::filesystem::path(ini.steamCMD).parent_path().string();
            processInit.installDir = steamCMDRoot + "/steamapps/" + appInfo.getValue("installDir", ini.appId);
            processInit.manifestPath = steamCMDRoot + "/steamapps/steamapps";
        }
        else
            processInit.manifestPath = ini.installDir + "/steamapps";

        processInit.manifestPath = processInit.manifestPath + "/appmanifest_" + processInit.appId + ".acf";
        if (!boost::filesystem::exists(processInit.manifestPath) || boost::filesystem::is_directory(processInit.manifestPath))
            throw file_find_exception(processInit.manifestPath);


        //create SteamCMD arguments
        std::string steamCMDArgs = " +login anonymous";
        if (!ini.installDir.empty())
            steamCMDArgs += " +force_install_dir " + ini.installDir;
        steamCMDArgs += " +app_update " + ini.appId;
        if (!ini.branch.empty())
            steamCMDArgs += " -beta " + ini.branch;
        steamCMDArgs += " +quit";

        // create update function
        auto update = [&ini, &steamCMDArgs]()
        {
            std::cout << "update Server" << std::endl;
            std::cout << "execute: " << ini.steamCMD << steamCMDArgs << std::endl;
            auto c = boost::process::execute(
                boost::process::initializers::run_exe(ini.steamCMD),
                boost::process::initializers::set_cmd_line(ini.steamCMD + steamCMDArgs),
                boost::process::initializers::start_in_dir(boost::filesystem::path(ini.steamCMD).parent_path())
            );
            boost::process::wait_for_exit(c);
        };

        // initial update event. Prevents not installed version of the appId thingy
        std::cout << "Check app for the first time, install if not installed\n";
        update();
        appInfo.updateCache();

        // Create server process
        SteamServerProcess p(appInfo, processInit);
        runServer(appInfo, p, ini.updateInterval, update);
        
    }
    catch (file_find_exception &e)
    {
        std::string func = *boost::get_error_info<file_find_exception::func_desc>(e);
        if (!func.empty())
            func += " : ";
        std::cerr << "Could not find file: " << func << *boost::get_error_info<boost::errinfo_file_name>(e) << std::endl;
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Program crash.\nPrinting backtrace:\n" << boost::current_exception_diagnostic_information() << std::endl;
    }
#ifdef _DEBUG && _MS_VER
    system("Pause");
#endif

}
