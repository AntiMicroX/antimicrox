#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QFileInfo>

#include "config.h"
#include "antimicrosettings.h"

#ifdef Q_OS_WIN

static QString findWinSystemConfigPath()
{
    QString temp;
    temp = (!qgetenv("LocalAppData").isEmpty()) ?
                QString(qgetenv("LocalAppData")) + "/antimicro" :
                QDir::homePath() + "/.antimicro";
    return temp;
}

static QString findWinLocalConfigPath()
{
    QString temp = QDir::currentPath();
    return temp;
}

static QString findWinDefaultConfigPath()
{
    QString temp = findWinLocalConfigPath();
    QFileInfo dirInfo(temp);
    if (!dirInfo.isWritable())
    {
        temp = findWinSystemConfigPath();
    }

    return temp;
}

static QString findWinConfigPath(QString configFileName)
{
    QString temp;
    QFileInfo localConfigInfo(findWinLocalConfigPath().append("/").append(configFileName));
    QFileInfo systemConfigInfo(findWinSystemConfigPath().append("/").append(configFileName));
    if (localConfigInfo.exists() && localConfigInfo.isWritable())
    {
        temp = localConfigInfo.absoluteFilePath();
    }
    else if (systemConfigInfo.exists() && systemConfigInfo.isWritable())
    {
        temp = systemConfigInfo.absoluteFilePath();
    }
    else
    {
        temp = findWinDefaultConfigPath().append("/").append(configFileName);
    }

    return temp;
}

#endif


namespace PadderCommon
{
#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    const QString configPath = findWinLocalConfigPath();
#elif defined(Q_OS_WIN)
    const QString configPath = findWinSystemConfigPath();
#else
    const QString configPath = (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
                QString(qgetenv("XDG_CONFIG_HOME")) + "/antimicro" :
                QDir::homePath() + "/.config/antimicro";

#endif

    const QString configFileName = "antimicro_settings.ini";
#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    const QString configFilePath = QString(configPath).append("/").append(configFileName);
#elif defined(Q_OS_WIN)
    const QString configFilePath = QString(configPath).append("/").append(configFileName);
#else
    const QString configFilePath = QString(configPath).append("/").append(configFileName);
#endif

    const int LATESTCONFIGFILEVERSION = 17;
    // Specify the last known profile version that requires a migration
    // to be performed in order to be compatible with the latest version.
    const int LATESTCONFIGMIGRATIONVERSION = 5;
    const QString localSocketKey = "antimicroSignalListener";
    const QString githubProjectPage = "https://github.com/Ryochan7/antimicro";
    const QString wikiPage = QString("%1/wiki").arg(githubProjectPage);
    const QString mouseDeviceName("antimicro Mouse Emulation");
    const QString keyboardDeviceName("antimicro Keyboard Emulation");
    const int ANTIMICRO_MAJOR_VERSION = PROJECT_MAJOR_VERSION;
    const int ANTIMICRO_MINOR_VERSION = PROJECT_MINOR_VERSION;
    const int ANTIMICRO_PATCH_VERSION = PROJECT_PATCH_VERSION;

    const QString programVersion = (ANTIMICRO_PATCH_VERSION > 0) ?
        QString("%1.%2.%3").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION).arg(ANTIMICRO_PATCH_VERSION) :
        QString("%1.%2").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION);

    QString preferredProfileDir(AntiMicroSettings *settings);
    QStringList arguments(int &argc, char **argv);
}

#endif // COMMON_H
