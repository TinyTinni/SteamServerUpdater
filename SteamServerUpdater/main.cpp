#include <iostream>
#include <functional>

#include "SteamServerProcess.h"
#include <boost/process.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/filesystem.hpp>

#include <boost/exception/all.hpp>

struct IniStruct
{
    std::string appId;
    std::string installDir;
    std::string args;
    std::string branch;
    std::string branchPW;

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
        initStruct.branchPW = pt.get<std::string>("AppInfo.BranchPW"); 

        initStruct.installDir = pt.get<std::string>("AppInfo.InstallDir");

        initStruct.steamCMD = pt.get<std::string>("General.SteamCMDBin");
        if (!boost::filesystem::exists(initStruct.steamCMD) || boost::filesystem::is_directory(initStruct.steamCMD))
            throw SteamServerProcess::file_find_exception(initStruct.steamCMD, "Steam Binary in .ini file: ");

        initStruct.updateInterval = pt.get<size_t>("General.UpdateInterval");

        return initStruct;
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error(std::string("Mailformatted .ini file: ") + e.what());
    }
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
        processInit.branchPW = ini.branchPW;
        processInit.steamCMD = ini.steamCMD;
        

        // extract manifest path for given appId and define installDir
        processInit.installDir = ini.installDir;
        

        // Create server process
        SteamServerProcess p(appInfo, std::move(processInit));
        p.run(ini.updateInterval);
        
    }
    catch (SteamServerProcess::file_find_exception &e)
    {
        std::string func = *boost::get_error_info<SteamServerProcess::file_find_exception::func_desc>(e);
        if (!func.empty())
            func += " : ";
        std::cerr << "[ERROR] Could not find file: " << func << " Given file: " << *boost::get_error_info<boost::errinfo_file_name>(e) << std::endl;
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "[ERROR] Runtime error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[ERROR] Program crash.\nPrinting backtrace:\n" << boost::current_exception_diagnostic_information() << std::endl;
    }
#ifdef _DEBUG && _MSC_VER
    system("Pause");
#endif

}
