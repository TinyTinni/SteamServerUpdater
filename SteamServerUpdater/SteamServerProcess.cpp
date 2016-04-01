#include "SteamServerProcess.h"

#include <QTextStream>

#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

QString SteamServerProcess::getInstalledVersion()
{
    QFile file(QString("%1%2appmanifest_%3.acf").arg(m_steamManifestDir.absolutePath(), QDir::separator(), m_appID));
    if (!file.open(QIODevice::ReadOnly))
        return "not open";
   
    //the vcf files looks like Json, but missing a ':' :/
    //QJsonParseError jerr;
    //QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &jerr);
    //if (jerr.error != QJsonParseError::NoError)
    //    return jerr.errorString(); //todo: error handline

    //QJsonObject obj = doc.object();
    //QString installedVersion = obj["buildid"].toString();

    QTextStream stream(&file);
    QString line = "";
    while (!line.contains("buildid") && !stream.atEnd())
    {
        stream.readLineInto(&line);
    }

    line = line.right(line.indexOf("\"") + QString("buildid\"").size()).trimmed();
    //line.remove("\"");
    return line.mid(1, line.size() - 2);
}