#pragma once

// SteamAppInfo
// recive information about a given app info#
// this class reads the cache data of SteamCMD due to the pipe "bug"? of SteamCMD where you
// are not able to pipe the output into a buffer
// ticket of the problem: https://github.com/ValveSoftware/Source-1-Games/issues/1929

#include <string>
#include <unordered_map>
#include <ctime>

class SteamAppInfo
{
#ifdef _MSC_VER
    typedef wchar_t unichar_t;
#else
    typedef char unichar_t;
#endif
    typedef std::basic_string<unichar_t> uniString;
    const static unichar_t spaceChar = unichar_t(0); // defines the char between 2 words in the appcache

    const std::string m_steamBin;

    const std::string m_appInfoPath; /// path to the cache file
    mutable std::time_t m_lastModified; /// last modification of the cache file
    mutable uniString m_cacheString; /// appInfo file content

    mutable std::unordered_map<std::string, size_t> m_appIdPosMap; /// caches the position of the appId in the current cache, please use the function getAppPos
    mutable std::unordered_map<std::string, std::string> m_appIdBuildId;  /// maps appId to last buildId
public:
    SteamAppInfo(const std::string steamCMDBin);
    ~SteamAppInfo();

    /// get the build id from appInfo. Returns empty string if appId wasnt found
    std::string getBuildId(const std::string& appId) const;

    /** get information from appInfo about a specified field (e.g. executable returns the relative path to the executable)
    returns empty string if nothing was found
    */
    std::string getValue(std::string key, const std::string& appId) const;

    /// updates the cache with steamCMD, you have to call it
    bool updateCache() const; //todo sync it with a given timer and call it automatically

private:

    /// search the cache file from given position for the specified number (e.g. a app id). (whle word mode)
    size_t searchCacheForNumber(unsigned appIdNumTmp, size_t pos = 0) const;

    /// search the cache file from the given position for the specified string (whole word mode)
    size_t searchCacheForString(std::string str, size_t pos = 0) const;

    /// get starting position in the cache file of. return nopos if appId wasnt found
    size_t getAppPos(const std::string& appId) const;
};