First version of the saturday evening/sunday morning program for a automatic steam server updater.
I updated the version, cleaned it up, more error handling if you cannot handle file paths etc.
Still alpha version because i need to test it, but it can be used.

# Steam Server Auto Updater
Steam doesn't provide a automatic update functionality if the application is running.
This is fine until you run a server or use [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD).
There is already a Auto Updater Script provided [here](https://github.com/C0nw0nk/SteamCMD-AutoUpdate-Any-Gameserver) which is linked in the wiki but it has two major drawbacks for me. First, its just VB and second, it uses the SteamWebAPI where you need to generate a key. The SteamWebAPI needs to be supported by the provides of the Serverbinary, otherwise it will give you not the right information.

So, I wrote my own updater, based on C++. All you need is SteamCMD, a internet connection and enough space for the server files.

## How to use?
You need a "ServerInfo.ini" file providing some information.
You can find an example in the current repo.
Every instance of the SteamServerUpdater can only run one server instance. This might change in the future.

## How Does it Work?
This program will work on the buildID. The buildID of your current installation is provided by the manifest data in "steamapps".
The current buildID of your application on the steamservers is saved within SteamCMD.
Since you cannot pipe from SteamCMD [(bugtracking)](https://github.com/ValveSoftware/Source-1-Games/issues/1929) you cannot ask SteamCMD for the buildID, but it is possible to read the buildID from the SteamCMD cache.

## Supported Platforms
Windows only. On Linux, you can pipe the output.

## Build Dependencies
- [CMake](https://cmake.org/) to generate make files
- [Boost](http://www.boost.org/)
- [Boost Process v0.5](http://www.highscore.de/boost/process0.5/)

## License

[MIT License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
