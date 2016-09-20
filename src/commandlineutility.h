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

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QObject>
#include <QStringList>
#include <QRegExp>
#include <QList>

#include "logger.h"

class ControllerOptionsInfo {
public:
    ControllerOptionsInfo()
    {
        controllerNumber = 0;
        startSetNumber = 0;
        unloadProfile = false;
    }

    bool hasProfile()
    {
        return !profileLocation.isEmpty();
    }

    QString getProfileLocation()
    {
        return profileLocation;
    }

    void setProfileLocation(QString location)
    {
        profileLocation = location;
    }

    bool hasControllerNumber()
    {
        return (controllerNumber > 0);
    }

    unsigned int getControllerNumber()
    {
        return controllerNumber;
    }

    void setControllerNumber(unsigned int temp)
    {
        controllerNumber = temp;
    }

    bool hasControllerID()
    {
        return !controllerIDString.isEmpty();
    }

    QString getControllerID()
    {
        return controllerIDString;
    }

    void setControllerID(QString temp)
    {
        controllerIDString = temp;
    }

    bool isUnloadRequested()
    {
        return unloadProfile;
    }

    void setUnloadRequest(bool status)
    {
        unloadProfile = status;
    }

    unsigned int getStartSetNumber()
    {
        return startSetNumber;
    }

    unsigned int getJoyStartSetNumber()
    {
        return startSetNumber - 1;
    }

    void setStartSetNumber(unsigned int temp)
    {
        if (temp >= 1 && temp <= 8)
        {
            startSetNumber = temp;
        }
    }

protected:
    QString profileLocation;
    unsigned int controllerNumber;
    QString controllerIDString;
    unsigned int startSetNumber;
    bool unloadProfile;
};

class CommandLineUtility : public QObject
{
    Q_OBJECT
public:
    explicit CommandLineUtility(QObject *parent = 0);

    void parseArguments(QStringList &arguments);
    bool isLaunchInTrayEnabled();
    bool isHelpRequested();
    bool isVersionRequested();
    bool isTrayHidden();
    bool hasProfile();
    bool hasControllerNumber();
    bool hasControllerID();

    QString getProfileLocation();

    unsigned int getControllerNumber();

    QString getControllerID();

    bool isHiddenRequested();
    bool isUnloadRequested();
    bool shouldListControllers();
    bool shouldMapController();

    unsigned int getStartSetNumber();
    unsigned int getJoyStartSetNumber();
    QList<unsigned int>* getJoyStartSetNumberList();

    QList<ControllerOptionsInfo>* getControllerOptionsList();
    bool hasProfileInOptions();

    QString getEventGenerator();

#ifdef Q_OS_UNIX
    bool launchAsDaemon();
    QString getDisplayString();
#endif

    void printHelp();
    void printVersionString();

    QString generateHelpString();
    QString generateVersionString();

    bool hasError();
    Logger::LogLevel getCurrentLogLevel();
    QString getCurrentLogFile();
    QString getErrorText();

protected:
    bool isPossibleCommand(QString temp);
    void setErrorMessage(QString temp);

    bool launchInTray;
    bool helpRequest;
    bool versionRequest;
    bool hideTrayIcon;

    QString profileLocation;
    unsigned int controllerNumber;
    QString controllerIDString;

    bool encounteredError;
    bool hiddenRequest;
    bool unloadProfile;

    unsigned int startSetNumber;

    bool daemonMode;
    QString displayString;
    bool listControllers;
    bool mappingController;
    QString eventGenerator;
    QString errorText;
    Logger::LogLevel currentLogLevel;
    QString currentLogFile;
    unsigned int currentListsIndex;
    QList<ControllerOptionsInfo> controllerOptionsList;

    static QRegExp trayRegexp;
    static QRegExp helpRegexp;
    static QRegExp versionRegexp;
    static QRegExp noTrayRegexp;
    static QRegExp loadProfileRegexp;
    static QRegExp loadProfileForControllerRegexp;
    static QRegExp hiddenRegexp;
    static QRegExp unloadRegexp;
    static QRegExp startSetRegexp;
    static QRegExp gamepadListRegexp;
    static QRegExp mappingRegexp;
    static QRegExp qtStyleRegexp;
    static QRegExp logLevelRegexp;
    static QRegExp logFileRegexp;
    static QRegExp eventgenRegexp;
    static QRegExp nextRegexp;
    static QStringList eventGeneratorsList;

#ifdef Q_OS_UNIX
    static QRegExp daemonRegexp;
    static QRegExp displayRegexp;
#endif
    
signals:
    
public slots:
    
};

#endif // COMMANDLINEPARSER_H
