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


#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

class QCommandLineParser;

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

    int getControllerNumber()
    {
        return controllerNumber;
    }

    void setControllerNumber(int temp)
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

    int getStartSetNumber()
    {
        return startSetNumber;
    }

    int getJoyStartSetNumber()
    {
        return startSetNumber - 1;
    }

    void setStartSetNumber(int temp)
    {
        if ((temp >= 1) && (temp <= 8))
        {
            startSetNumber = temp;
        }
    }

private:
    QString profileLocation;
    int controllerNumber;
    QString controllerIDString;
    int startSetNumber;
    bool unloadProfile;
};

class CommandLineUtility : public QObject
{
    Q_OBJECT

public:
    explicit CommandLineUtility(QObject *parent = nullptr);

    void parseArguments(QCommandLineParser* parser);

    bool isLaunchInTrayEnabled();
    bool isTrayHidden();
    bool hasProfile();
    bool hasControllerNumber();
    bool hasControllerID();
    bool isHiddenRequested();
    bool isUnloadRequested();
    bool shouldListControllers();
    bool shouldMapController();
    bool hasProfileInOptions();
    bool hasError();

    int getControllerNumber();
    int getStartSetNumber();
    int getJoyStartSetNumber();

    QString getControllerID();
    QString getProfileLocation();
    QString getEventGenerator();
    QString getCurrentLogFile();
    QString getErrorText();

    QList<int>* getJoyStartSetNumberList();
    QList<ControllerOptionsInfo> const& getControllerOptionsList();

    bool launchAsDaemon();
    QString getDisplayString();

    Logger::LogLevel getCurrentLogLevel();

protected:
    void setErrorMessage(QString temp);

private:
    bool launchInTray;
    bool hideTrayIcon;
    bool encounteredError;
    bool hiddenRequest;
    bool unloadProfile;
    bool daemonMode;
    bool listControllers;
    bool mappingController;

    int startSetNumber;
    int controllerNumber;
    int currentListsIndex;

    QString profileLocation;
    QString controllerIDString;
    QString displayString;
    QString eventGenerator;
    QString errorText;
    QString currentLogFile;

    Logger::LogLevel currentLogLevel;

    QList<ControllerOptionsInfo> controllerOptionsList;

    static QStringList eventGeneratorsList;

    void parseArgsProfile(QCommandLineParser* parser);
    void parseArgsPrControle(QCommandLineParser* parser);
    void parseArgsUnload(QCommandLineParser* parser);
    void parseArgsStartSet(QCommandLineParser* parser);
    void parseArgsMap(QCommandLineParser* parser);
    
};

#endif // COMMANDLINEPARSER_H
