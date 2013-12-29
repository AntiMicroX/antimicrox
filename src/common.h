#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QSettings>

namespace PadderCommon
{
#if defined(Q_OS_WIN)
const QString configPath = (!qgetenv("LocalAppData").isEmpty()) ?
            QString(qgetenv("LocalAppData")) + "/antimicro" :
            QDir::homePath() + "/.antimicro";

#else
    const QString configPath = (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
                QString(qgetenv("XDG_CONFIG_HOME")) + "/antimicro" :
                QDir::homePath() + "/.config/antimicro";

#endif

    const QString configFileName = "antimicro_settings.ini";
    const QString controllerMappingFileName = "controller_mappings.ini";
    const QString configFilePath = configPath + "/" + configFileName;
    const QString controllerMappingFilePath = configPath + "/" + controllerMappingFileName;
    const int LATESTCONFIGFILEVERSION = 5;
    const QString programVersion = "1.2";
    const QString localSocketKey = "antimicroSignalListener";
}

#endif // COMMON_H
