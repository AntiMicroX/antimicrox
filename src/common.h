/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QTranslator>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>
#include <QThread>
#include <QCoreApplication>

#include "config.h"
#include "antimicrosettings.h"
#include "mousehelper.h"

#ifdef Q_OS_WIN

static QString findWinSystemConfigPath()
{
    QString temp;
    temp = (!qgetenv("LocalAppData").isEmpty()) ?
                QString::fromUtf8(qgetenv("LocalAppData")) + "/antimicro" :
                QDir::homePath() + "/.antimicro";
    return temp;
}

static QString findWinLocalConfigPath()
{
  QString temp = QCoreApplication::applicationDirPath();
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
  inline QString configPath() {
#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    return findWinLocalConfigPath();
#elif defined(Q_OS_WIN)
    return findWinSystemConfigPath();
#else
    return  (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
      QString::fromUtf8(qgetenv("XDG_CONFIG_HOME")) + "/antimicro" :
      QDir::homePath() + "/.config/antimicro";
#endif
  }

  const QString configFileName = "antimicro_settings.ini";
  inline QString configFilePath() {
#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    return QString(configPath()).append("/").append(configFileName);
#elif defined(Q_OS_WIN)
    return QString(configPath()).append("/").append(configFileName);
#else
    return QString(configPath()).append("/").append(configFileName);
#endif
  }
  
    const int LATESTCONFIGFILEVERSION = 19;
    // Specify the last known profile version that requires a migration
    // to be performed in order to be compatible with the latest version.
    const int LATESTCONFIGMIGRATIONVERSION = 5;
    const QString localSocketKey = "antimicroSignalListener";
    const QString githubProjectPage = "https://github.com/AntiMicro/antimicro";
    const QString wikiPage = QString("%1/wiki").arg(githubProjectPage);

    const QString mouseDeviceName("antimicro Mouse Emulation");
    const QString keyboardDeviceName("antimicro Keyboard Emulation");
    const QString springMouseDeviceName("antimicro Abs Mouse Emulation");
    
    const int ANTIMICRO_MAJOR_VERSION = PROJECT_MAJOR_VERSION;
    const int ANTIMICRO_MINOR_VERSION = PROJECT_MINOR_VERSION;
    const int ANTIMICRO_PATCH_VERSION = PROJECT_PATCH_VERSION;

    const QString programVersion = (ANTIMICRO_PATCH_VERSION > 0) ?
        QString("%1.%2.%3").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION).arg(ANTIMICRO_PATCH_VERSION) :
        QString("%1.%2").arg(ANTIMICRO_MAJOR_VERSION)
            .arg(ANTIMICRO_MINOR_VERSION);

    extern QWaitCondition waitThisOut;
    extern QMutex sdlWaitMutex;
    extern QMutex inputDaemonMutex;
    extern bool editingBindings;
    extern QReadWriteLock editingLock;
    extern MouseHelper mouseHelperObj;

    QString preferredProfileDir(AntiMicroSettings *settings);
    QStringList arguments(int &argc, char **argv);
    QStringList parseArgumentsString(QString tempString);
    void reloadTranslations(QTranslator *translator,
                            QTranslator *appTranslator,
                            QString language);
    void lockInputDevices();
    void unlockInputDevices();

    /*!
     * \brief Returns the "human-readable" name of the given profile.
     */
    inline QString getProfileName(QFileInfo& profile) {
      QString retVal = profile.completeBaseName();

      return retVal;
    }
}

Q_DECLARE_METATYPE(QThread*)

#endif // COMMON_H
