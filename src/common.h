#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QSettings>

namespace PadderCommon
{
    const QString configPath = (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
                QString(qgetenv("XDG_CONFIG_HOME")) + "/antimicro" :
                QDir::homePath() + "/.config/antimicro";
    const QString configFileName = "antimicro_settings.ini";
    const QString configFilePath = configPath + "/" + configFileName;
    const QString pidFilePath = "/tmp/antimicro.pid";
    const int LATESTCONFIGFILEVERSION = 3;
    const QString programVersion = "0.7.9";
}

#endif // COMMON_H
