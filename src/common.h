#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDir>
#include <QSettings>

namespace PadderCommon
{
    const QString configPath = QDir::homePath() + "/.antimicro";
    const QString configFileName = "antimicro_settings.ini";
    const QString configFilePath = configPath + "/" + configFileName;
    const int LATESTCONFIGFILEVERSION = 2;
    const QString programVersion = "0.7";
}

#endif // COMMON_H
