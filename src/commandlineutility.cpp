/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "commandlineutility.h"

#include "messagehandler.h"
#include "common.h"
#include "eventhandlerfactory.h"

#include <QDebug>
#include <QFileInfo>
#include <QCommandLineParser>
#include <QApplication>

QStringList CommandLineUtility::eventGeneratorsList = EventHandlerFactory::buildEventGeneratorList();

CommandLineUtility::CommandLineUtility(QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    launchInTray = false;
    hideTrayIcon = false;
    profileLocation = "";
    controllerNumber = 0;
    encounteredError = false;
    hiddenRequest = false;
    unloadProfile = false;
    startSetNumber = 0;
    daemonMode = false;
    displayString = "";
    listControllers = false;
    mappingController = false;
    currentLogLevel = Logger::LOG_NONE;

    currentListsIndex = 0;
    ControllerOptionsInfo tempInfo;
    controllerOptionsList.append(tempInfo);

    eventGenerator = EventHandlerFactory::fallBackIdentifier();
}


void CommandLineUtility::parseArguments(QCommandLineParser* parser) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int i = 0;

    while ((i < parser->optionNames().count()) && !encounteredError)
    {
        if (parser->isSet("tray"))
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "tray is set";
            #endif

            launchInTray = true;
            hideTrayIcon = false;
        }
        else if (parser->isSet("no-tray"))
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "no-tray is set";
            #endif

            hideTrayIcon = true;
            launchInTray = false;
        }

        if (parser->isSet("profile"))
        {
            parseArgsProfile(parser);
        }

        if (parser->isSet("profile-controller"))
        {
            parseArgsPrControle(parser);
        }

        if (parser->isSet("hidden"))
        {
            hiddenRequest = true;
        }

        if (parser->isSet("unload"))
        {
            parseArgsUnload(parser);
        }

        if (parser->isSet("startSet"))
        {
            parseArgsStartSet(parser);
        }

        if (parser->isSet("next"))
        {
            currentListsIndex++;
            ControllerOptionsInfo tempInfo;
            controllerOptionsList.append(tempInfo);
        }

        if (parser->isSet("list"))
        {
            listControllers = true;
        }

        if (parser->isSet("map"))
        {
            parseArgsMap(parser);
        }

        if (parser->isSet("daemon"))
        {
            daemonMode = true;
        }
        #ifdef WITH_X11
        if (parser->isSet("display"))
        {
            if (!parser->value("display").isEmpty())
            {
                displayString = parser->value("display");
            }
            else
            {
                setErrorMessage(QObject::tr("No display string was specified."));
            }
        }
        #endif

    #if (defined(WITH_UINPUT) && defined(WITH_XTEST))

        if (parser->isSet("eventgen"))
        {
           QString eventGenText = parser->value("eventgen");

           if (!eventGenText.isEmpty() && !eventGeneratorsList.contains(eventGenText))
           {
               eventGenerator = "";
               setErrorMessage(QObject::tr("An invalid event generator was specified."));
            }
            else if (!eventGenText.isEmpty() && eventGeneratorsList.contains(eventGenText))
            {
                eventGenerator = eventGenText;
            }
            else
            {
                setErrorMessage(QObject::tr("No event generator string was specified."));
            }
        }
    #endif

        if (parser->isSet("log-level"))
        {
            QString logLevelText = parser->value("log-level");

            if (logLevelText == "debug") currentLogLevel = Logger::LOG_DEBUG;
            else if (logLevelText == "info") currentLogLevel = Logger::LOG_INFO;
            else if (logLevelText == "warn") currentLogLevel = Logger::LOG_WARNING;
            else if (logLevelText == "error") currentLogLevel = Logger::LOG_ERROR;
        }

        if (parser->isSet("log-file"))
        {
            if (!parser->value("log-file").isEmpty())
            {
                currentLogFile = parser->value("log-file");
            }
            else
            {
                setErrorMessage(QObject::tr("No log file specified."));
            }
        }

        i++;
    }
}

void CommandLineUtility::parseArgsProfile(QCommandLineParser* parser)
{
    QFileInfo profileFileInfo(parser->value("profile"));

    if (profileFileInfo.exists() && (profileFileInfo.suffix() != "amgp") && (profileFileInfo.suffix() != "xml"))
    {
        setErrorMessage(QObject::tr("Profile location %1 is not an XML file.").arg(parser->value("profile")));
    }
    else if (profileFileInfo.exists() && ((profileFileInfo.suffix() == "amgp") || (profileFileInfo.suffix() == "xml")))
    {
        QString tempProfileLocation = profileFileInfo.absoluteFilePath();
        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setProfileLocation(tempProfileLocation);
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    }
    else
    {
        setErrorMessage(QObject::tr("Profile location %1 does not exist.").arg(parser->value("profile")));
    }
}

void CommandLineUtility::parseArgsPrControle(QCommandLineParser* parser)
{
    bool validNumber = false;
    int tempNumber = parser->value("profile-controller").toInt(&validNumber);

    if (validNumber)
    {
        if (controllerNumber == 0) controllerNumber = tempNumber;

        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setControllerNumber(tempNumber);
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    }
    else if (!parser->value("profile-controller").isEmpty())
    {
        if (controllerIDString.isEmpty()) controllerIDString = parser->value("profile-controller");

        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setControllerID(parser->value("profile-controller"));
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    }
    else
    {
        setErrorMessage(QObject::tr("Controller identifier is not a valid value."));
    }
}

void CommandLineUtility::parseArgsUnload(QCommandLineParser* parser)
{
    ControllerOptionsInfo gameControllerOptionInfo = getControllerOptionsList().at(currentListsIndex);
    gameControllerOptionInfo.setProfileLocation("");
    gameControllerOptionInfo.setUnloadRequest(true);
    controllerOptionsList.replace(currentListsIndex, gameControllerOptionInfo);

    QStringList parserVals = QStringList() << parser->values("unload");
    QListIterator<QString> profileForUnload(parserVals);
    QString unloadedProfText = QString();

    if (profileForUnload.hasNext())
    {
        unloadedProfText = profileForUnload.next();

        // A value has been passed. Attempt
        // to validate the value.

        bool validNumber = false;
        int tempNumber = unloadedProfText.toInt(&validNumber);

        if (validNumber)
        {
            controllerNumber = tempNumber;
            gameControllerOptionInfo = getControllerOptionsList().at(currentListsIndex);
            gameControllerOptionInfo.setControllerNumber(controllerNumber);
            controllerOptionsList.replace(currentListsIndex, gameControllerOptionInfo);
        }
        else if (!unloadedProfText.isEmpty())
        {
            controllerIDString = unloadedProfText;
            gameControllerOptionInfo = getControllerOptionsList().at(currentListsIndex);
            gameControllerOptionInfo.setControllerID(controllerIDString);
            controllerOptionsList.replace(currentListsIndex, gameControllerOptionInfo);
        }
        else
        {
            setErrorMessage(QObject::tr("Controller identifier is not a valid value."));
        }
    }
    else
    {
        unloadProfile = true;
        profileLocation = "";
    }
}

void CommandLineUtility::parseArgsStartSet(QCommandLineParser* parser)
{
    QStringList parserValues = QStringList() << parser->values("startSet");
    QListIterator<QString> profileForStartset(parserValues);
    QString startSetText = QString();

    if (profileForStartset.hasNext())
    {
        startSetText = profileForStartset.next();

        bool validNumber = false;
        int tempNumber = startSetText.toInt(&validNumber);

        if (validNumber && (tempNumber >= 1) && (tempNumber <= 8))
        {
            startSetNumber = tempNumber;
            ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
            tempInfo.setStartSetNumber(startSetNumber);
            controllerOptionsList.replace(currentListsIndex, tempInfo);
        }
        else if (validNumber)
        {
            setErrorMessage(QObject::tr("An invalid set number '%1' was specified.").arg(tempNumber));
        }

        if (profileForStartset.hasNext())
        {
            startSetText = profileForStartset.next();

            if (validNumber)
            {
                controllerNumber = tempNumber;
                ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                tempInfo.setControllerNumber(controllerNumber);
                controllerOptionsList.replace(currentListsIndex, tempInfo);
            }
            else if (!startSetText.isEmpty())
            {
                controllerIDString = startSetText;
                ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                tempInfo.setControllerID(controllerIDString);
                controllerOptionsList.replace(currentListsIndex, tempInfo);
            }
            else
            {
                setErrorMessage(QObject::tr("Controller identifier '%s'' is not a valid value.").arg(startSetText));
            }
        }
    }
    else
    {
        setErrorMessage(QObject::tr("No set number was specified."));
    }
}

void CommandLineUtility::parseArgsMap(QCommandLineParser* parser)
{
    QString mapOptionText = parser->value("map");

    if (!mapOptionText.isEmpty()) {

        bool validNumber = false;
        int tempNumber = mapOptionText.toInt(&validNumber);

        if (validNumber)
        {
            controllerNumber = tempNumber;
            mappingController = true;
        }
        else if (!mapOptionText.isEmpty())
        {
            controllerIDString = mapOptionText;
            mappingController = true;
        }
        else
        {
            setErrorMessage(QObject::tr("Controller identifier is not a valid value."));
        }
    }
    else
    {
        setErrorMessage(QObject::tr("No controller was specified."));
    }
}

bool CommandLineUtility::isLaunchInTrayEnabled()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return launchInTray;
}

bool CommandLineUtility::isTrayHidden()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return hideTrayIcon;
}

bool CommandLineUtility::hasProfile()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return !profileLocation.isEmpty();
}

bool CommandLineUtility::hasControllerNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (controllerNumber > 0);
}

QString CommandLineUtility::getProfileLocation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return profileLocation;
}

int CommandLineUtility::getControllerNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return controllerNumber;
}

bool CommandLineUtility::hasError()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return encounteredError;
}

bool CommandLineUtility::isHiddenRequested()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return hiddenRequest;
}

bool CommandLineUtility::hasControllerID()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return !controllerIDString.isEmpty();
}

QString CommandLineUtility::getControllerID()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return controllerIDString;
}

bool CommandLineUtility::isUnloadRequested()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return unloadProfile;
}

int CommandLineUtility::getStartSetNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return startSetNumber;
}

int CommandLineUtility::getJoyStartSetNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return startSetNumber - 1;
}

bool CommandLineUtility::shouldListControllers()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return listControllers;
}

bool CommandLineUtility::shouldMapController()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mappingController;
}

QString CommandLineUtility::getEventGenerator()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return eventGenerator;
}


bool CommandLineUtility::launchAsDaemon()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return daemonMode;
}

QString CommandLineUtility::getDisplayString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return displayString;
}


Logger::LogLevel CommandLineUtility::getCurrentLogLevel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentLogLevel;
}

QString CommandLineUtility::getCurrentLogFile() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentLogFile;
}

QString CommandLineUtility::getErrorText() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return errorText;
}

void CommandLineUtility::setErrorMessage(QString temp)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    errorText = temp;
    encounteredError = true;
}

QList<ControllerOptionsInfo> const& CommandLineUtility::getControllerOptionsList()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return controllerOptionsList;
}

bool CommandLineUtility::hasProfileInOptions()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<ControllerOptionsInfo> iter(getControllerOptionsList());

    while (iter.hasNext())
    {
        ControllerOptionsInfo temp = iter.next();

        if (temp.hasProfile())
        {
            result = true;
            iter.toBack();
        }
    }

    return result;
}
