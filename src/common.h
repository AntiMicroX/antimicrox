/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "config.h"
#include "antimicrosettings.h"
#include "mousehelper.h"

#include <QDir>
#include <QThread>
#include <QTranslator>
#include <QWaitCondition>



namespace PadderCommon
{
  inline QString configPath() {

    return  (!qgetenv("XDG_CONFIG_HOME").isEmpty()) ?
      QString::fromUtf8(qgetenv("XDG_CONFIG_HOME")) + "/antimicroX" :
      QDir::homePath() + "/.config/antimicroX";
  }

  const QString configFileName = "antimicroX_settings.ini";
  inline QString configFilePath() {

  return QString(configPath()).append("/").append(configFileName);

  }
  
    const int LATESTCONFIGFILEVERSION = 19;
    // Specify the last known profile version that requires a migration
    // to be performed in order to be compatible with the latest version.
    const int LATESTCONFIGMIGRATIONVERSION = 5;
    const QString localSocketKey = "antimicroXSignalListener";
    const QString githubProjectPage = "https://github.com/juliagoda/antimicroX";
    const QString githubIssuesPage = "https://github.com/juliagoda/antimicroX/issues";
    const QString wikiPage = QString("%1/wiki").arg(githubProjectPage);

    const QString mouseDeviceName("antimicroX Mouse Emulation");
    const QString keyboardDeviceName("antimicroX Keyboard Emulation");
    const QString springMouseDeviceName("antimicroX Abs Mouse Emulation");
    
    const int ANTIMICROX_MAJOR_VERSION = PROJECT_MAJOR_VERSION;
    const int ANTIMICROX_MINOR_VERSION = PROJECT_MINOR_VERSION;
    const int ANTIMICROX_PATCH_VERSION = PROJECT_PATCH_VERSION;

    const QString programVersion = (ANTIMICROX_PATCH_VERSION > 0) ?
        QString("%1.%2.%3").arg(ANTIMICROX_MAJOR_VERSION)
            .arg(ANTIMICROX_MINOR_VERSION).arg(ANTIMICROX_PATCH_VERSION) :
        QString("%1.%2").arg(ANTIMICROX_MAJOR_VERSION)
            .arg(ANTIMICROX_MINOR_VERSION);

    extern QWaitCondition waitThisOut;
    extern QMutex sdlWaitMutex;
    extern QMutex inputDaemonMutex;
    extern bool editingBindings;
    extern QReadWriteLock editingLock;
    extern MouseHelper mouseHelperObj;

    QString preferredProfileDir(AntiMicroSettings *settings);
    QStringList arguments(const int &argc, char **argv);
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
