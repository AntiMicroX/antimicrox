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
}

#endif // COMMON_H
