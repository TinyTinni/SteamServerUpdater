#include <iostream>

#include "SteamServerProcess.h"
#include <boost/process.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

int main(int argc, char** argv)
{
    // init
    std::cout << "Read Ini";
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("ServerInfo.ini", pt);
    SteamServerProcess::Init initStruct;
    initStruct.appId = pt.get<std::string>("AppInfo.AppId");
    initStruct.args = pt.get<std::string>("AppInfo.Args");
    initStruct.installDir = pt.get<std::string>("AppInfo.InstallDir");
    //initStruct.manifestPath = pt.get<std::string>("AppInfo.ManifestPath");//todo: extract from given info
    const std::string steamCMDBin = pt.get<std::string>("General.SteamCMDBin");
    const size_t updateInterval = pt.get<size_t>("General.UpdateInterval");
    std::cout << " Done" << std::endl;

    // Read cache file
    std::cout << "Create AppInfo";
    SteamAppInfo appInfo(steamCMDBin);
    std::cout << " Done" << std::endl;
    if (initStruct.installDir.empty())
    {
        const std::string steamCMDRoot = boost::filesystem::path(steamCMDBin).parent_path().string();
        initStruct.installDir = steamCMDRoot + "/steamapps/" + appInfo.getValue("installDir", initStruct.appId);
        initStruct.manifestPath = steamCMDRoot + "/steamapps/steamapps"; //its steamapps/steamapps
    }
    else
        initStruct.manifestPath = initStruct.installDir + "/steamapps";
    

    //create SteamCMD arguments
    std::string steamCMDArgs = " +login anonymous";
    if (!initStruct.installDir.empty())
        steamCMDArgs += " +force_install_dir " + initStruct.installDir;
    steamCMDArgs += " +app_update " + initStruct.appId + " +quit";


    auto update = [&]() 
    {
        std::cout << "update Server" << std::endl;
        auto c = boost::process::execute(
            boost::process::initializers::run_exe(steamCMDBin),
            boost::process::initializers::set_cmd_line(steamCMDArgs),
            boost::process::initializers::start_in_dir(boost::filesystem::path(steamCMDBin).parent_path())
        );
        boost::process::wait_for_exit(c);
    };

    // initial update event. Prevents not installed version of the appId thingy
    std::cout << "Check app for the first time, install if not installed\n";
    update();
    appInfo.updateCache();

    // Create server process
    SteamServerProcess p(appInfo, initStruct);
    p.startProgram();
    size_t i = 0;
    while (p.isRunning())
    {
        p.waitFor(1000 * updateInterval);
        appInfo.updateCache();
        
        if (p.getInstalledBuildId() != appInfo.getBuildId(p.getAppId()))
        {
            std::cout << "update from: " << p.getInstalledBuildId() << " to: " << appInfo.getBuildId(p.getAppId()) << std::endl;
            p.stopProgram();
            update();
            p.startProgram();
            ++i;
        }
        
    }
    std::cout << "#Updates: " << i << std::endl;


    system("Pause");
}
