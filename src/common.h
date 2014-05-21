#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QSettings>
#include <QStringList>

#include "config.h"

namespace PadderCommon
{
#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    const QString configPath = QDir::currentPath();
#elif defined(Q_OS_WIN)
    const QString configPath = (!qgetenv("LocalAppData").isEmpty()) ?
            QString(qgetenv("LocalAppData")) + "/antimicro" :
            QDir::homePath() + "/.antimicro";

#else
    const QString configPath = (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
                QString(qgetenv("XDG_CONFIG_HOME")) + "/antimicro" :
                QDir::homePath() + "/.config/antimicro";

#endif

    const QString configFileName = "antimicro_settings.ini";
    const QString configFilePath = configPath + "/" + configFileName;
    const int LATESTCONFIGFILEVERSION = 8;
    // Specify the last known profile version that requires a migration
    // to be performed in order to be compatible with the latest version.
    const int LATESTCONFIGMIGRATIONVERSION = 5;
    const QString localSocketKey = "antimicroSignalListener";
    const QString projectHomePage = "http://ryochan7.com/projects/antimicro/";
    const QString githubProjectPage = "https://github.com/Ryochan7/antimicro";
    const int ANTIMICRO_MAJOR_VERSION = PROJECT_MAJOR_VERSION;
    const int ANTIMICRO_MINOR_VERSION = PROJECT_MINOR_VERSION;
    const int ANTIMICRO_PATCH_VERSION = PROJECT_PATCH_VERSION;

    const QString programVersion = (ANTIMICRO_PATCH_VERSION > 0) ?
        QString("%1.%2.%3").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION).arg(ANTIMICRO_PATCH_VERSION) :
        QString("%1.%2").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION);

    QString preferredProfileDir(QSettings *settings);
    QStringList arguments(int &argc, char **argv);
}

#endif // COMMON_H
