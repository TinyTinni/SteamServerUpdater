#pragma once

#include <QDir>



class SteamServerProcess
{
    const QString m_appID;
    const QDir m_steamManifestDir;
public:
    SteamServerProcess(const QString appID, const QDir steamManifestDir) :m_appID(appID), m_steamManifestDir(steamManifestDir) {};
    ~SteamServerProcess() {}
    QString getInstalledVersion();
};