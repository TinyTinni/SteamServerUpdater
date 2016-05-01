First version of the saturday evening/sunday morning program for a automatic steam server updater. Please don't kill me for the code mess, i will clean it up later.

# SteamServerUpdater
Steam doesn't provide a automatic update functionality if the application is running.
This is fine until you run a server or use [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD).
There is already a Auto Updater Script provided [here](https://github.com/C0nw0nk/SteamCMD-AutoUpdate-Any-Gameserver) which is linked in the wiki but it has two major drawbacks for me. First, its just VB and second, it uses the SteamWebAPI where you need to generate a key. The SteamWebAPI needs to be supported by the provides of the Serverbinary, otherwise it will give you not the right information.

So, I wrote my own updater, based on C++. All you need is SteamCMD, a internet connection and enough space for the server files.

#How to use?
You need a "ServerInfo.ini" file providing some information.
You can find an example in the current repo.
Please take into account, that error handling is not yet implemented. Thats means, it will crash if some paths provided are wrong.

#How Does it Work?
This program will work on the buildID. The buildID of your current installation is provided by the manifest data in "steamapps".
The current buildID of your application on the steamservers is saved within SteamCMD.
Since you cannot pipe from SteamCMD [(bugtracking)](https://github.com/ValveSoftware/Source-1-Games/issues/1929) you cannot ask SteamCMD for the buildID, but it is possible to read the buildID from the SteamCMD cache.

#Supported Platforms
It is possible to run the updater on every platform, but it was only tested on windows. On Linux, the paths of the files, like 'appinfo.vdf' are different.

#Build Dependencies
- [CMake](https://cmake.org/) to generate make files
- [Boost](http://www.boost.org/)
- [Boost Process v0.5](http://www.highscore.de/boost/process0.5/)

