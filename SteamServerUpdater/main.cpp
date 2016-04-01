#include <iostream>

#include "SteamServerProcess.h"

int main(int argc, char** argv)
{
    SteamServerProcess p("343050", QString("C:\\test\\steamapps"));
    std::cout << "installed Version: " << p.getInstalledVersion().toStdString() << std::endl;;
    
    system("Pause");
}