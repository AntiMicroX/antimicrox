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

#include "commandlineutility.h"

#include "messagehandler.h"
#include "common.h"
#include "eventhandlerfactory.h"

#include <QDebug>
#include <QFileInfo>
#include <QCommandLineParser>

#ifdef Q_OS_UNIX
#include <QApplication>
#endif

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
        else if (parser->isSet("profile"))
        {

            QFileInfo fileInfo(parser->value("profile"));
                if (fileInfo.exists())
                {
                    if ((fileInfo.suffix() != "amgp") && (fileInfo.suffix() != "xml"))
                    {
                        setErrorMessage(QObject::trUtf8("Profile location %1 is not an XML file.").arg(parser->value("profile")));
                    }
                    else
                    {
                        QString tempProfileLocation = fileInfo.absoluteFilePath();
                        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                        tempInfo.setProfileLocation(tempProfileLocation);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                }
                else
                {
                    setErrorMessage(QObject::trUtf8("Profile location %1 does not exist.").arg(parser->value("profile")));
                }
        }
        else if (parser->isSet("profile-controller"))
        {
                bool validNumber = false;
                int tempNumber = parser->value("profile-controller").toInt(&validNumber);
                if (validNumber)
                {
                    if (controllerNumber == 0)
                    {
                        controllerNumber = tempNumber;
                    }

                    ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                    tempInfo.setControllerNumber(tempNumber);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else if (!parser->value("profile-controller").isEmpty())
                {
                    if (controllerIDString.isEmpty())
                    {
                        controllerIDString = parser->value("profile-controller");
                    }

                    ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                    tempInfo.setControllerID(parser->value("profile-controller"));
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else
                {
                    setErrorMessage(QObject::trUtf8("Controller identifier is not a valid value."));
                }
        }
        else if (parser->isSet("hidden"))
        {
            hiddenRequest = true;
        }
        else if (parser->isSet("unload"))
        {
            ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
            tempInfo.setProfileLocation("");
            tempInfo.setUnloadRequest(true);
            controllerOptionsList.replace(currentListsIndex, tempInfo);

            QStringList values = QStringList() << parser->values("unload");
            QListIterator<QString> unloaded_profile(values);
            QString temp = QString();

            if (unloaded_profile.hasNext())
            {

                temp = unloaded_profile.next();

                    // A value has been passed. Attempt
                    // to validate the value.
                    bool validNumber = false;
                    int tempNumber = temp.toInt(&validNumber);
                    if (validNumber)
                    {
                        controllerNumber = tempNumber;
                        tempInfo = getControllerOptionsList().at(currentListsIndex);
                        tempInfo.setControllerNumber(controllerNumber);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                    else if (!temp.isEmpty())
                    {
                        controllerIDString = temp;
                        tempInfo = getControllerOptionsList().at(currentListsIndex);
                        tempInfo.setControllerID(controllerIDString);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                    else
                    {
                        setErrorMessage(QObject::trUtf8("Controller identifier is not a valid value."));
                    }

            }
            else
            {
                unloadProfile = true;
                profileLocation = "";
            }
        }
        else if (parser->isSet("startSet"))
        {
            QStringList values = QStringList() << parser->values("startSet");
            QListIterator<QString> unloaded_profile(values);
            QString temp = QString();

            if (unloaded_profile.hasNext())
            {
                temp = unloaded_profile.next();

                bool validNumber = false;
                int tempNumber = temp.toInt(&validNumber);
                if (validNumber && (tempNumber >= 1) && (tempNumber <= 8))
                {
                    startSetNumber = tempNumber;
                    ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                    tempInfo.setStartSetNumber(startSetNumber);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else if (validNumber)
                {
                    setErrorMessage(QObject::trUtf8("An invalid set number '%1' was specified.").arg(tempNumber));
                }

                if (unloaded_profile.hasNext())
                {
                    temp = unloaded_profile.next();

                        if (validNumber)
                        {
                            controllerNumber = tempNumber;
                            ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                            tempInfo.setControllerNumber(controllerNumber);
                            controllerOptionsList.replace(currentListsIndex, tempInfo);
                        }
                        else if (!temp.isEmpty())
                        {
                            controllerIDString = temp;
                            ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                            tempInfo.setControllerID(controllerIDString);
                            controllerOptionsList.replace(currentListsIndex, tempInfo);
                        }
                        else
                        {
                            setErrorMessage(QObject::trUtf8("Controller identifier '%s'' is not a valid value.").arg(temp));
                        }
                }
            }
            else
            {
                setErrorMessage(QObject::trUtf8("No set number was specified."));
            }
        }
        else if (parser->isSet("next"))
        {
            currentListsIndex++;

            ControllerOptionsInfo tempInfo;
            controllerOptionsList.append(tempInfo);

        }

#ifdef USE_SDL_2
        else if (parser->isSet("list"))
        {
            listControllers = true;
        }
        else if (parser->isSet("map"))
        {
                QString temp = parser->value("map");

         if (!temp.isEmpty()) {

                bool validNumber = false;
                int tempNumber = temp.toInt(&validNumber);
                if (validNumber)
                {
                    controllerNumber = tempNumber;
                    mappingController = true;
                }
                else if (!temp.isEmpty())
                {
                    controllerIDString = temp;
                    mappingController = true;
                }
                else
                {
                    setErrorMessage(QObject::trUtf8("Controller identifier is not a valid value."));
                }
            }
            else
            {
                setErrorMessage(QObject::trUtf8("No controller was specified."));
            }

        }
#endif

#ifdef Q_OS_UNIX
        else if (parser->isSet("daemon"))
        {
            daemonMode = true;
        }
  #ifdef WITH_X11
        else if (parser->isSet("display"))
        {
            if (!parser->value("display").isEmpty())
            {
                displayString = parser->value("display");
            }
            else
            {
                setErrorMessage(QObject::trUtf8("No display string was specified."));
            }
        }
  #endif
#endif

#if (defined (Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)) \
     || (defined(Q_OS_WIN) && defined(WITH_VMULTI))
        else if (parser->isSet("eventgen"))
        {
                QString temp = parser->value("eventgen");


           if (!temp.isEmpty()) {

                if (!eventGeneratorsList.contains(temp))
                {
                    eventGenerator = "";
                    setErrorMessage(QObject::trUtf8("An invalid event generator was specified."));
                }
                else
                {
                    eventGenerator = temp;
                }
            }
            else
            {
                setErrorMessage(QObject::trUtf8("No event generator string was specified."));
            }
        }

#endif

        else if (parser->isSet("log-level"))
        {
                QString temp = parser->value("log-level");
                if (temp == "debug")
                {
                    currentLogLevel = Logger::LOG_DEBUG;
                }
                else if (temp == "info")
                {
                    currentLogLevel = Logger::LOG_INFO;
                }
                else if (temp == "warn")
                {
                    currentLogLevel = Logger::LOG_WARNING;
                }
                else if (temp == "error")
                {
                    currentLogLevel = Logger::LOG_ERROR;
                }
        }
        else if (parser->isSet("log-file"))
        {
            if (!parser->value("log-file").isEmpty())
            {
                currentLogFile = parser->value("log-file");
            }
            else
            {
                setErrorMessage(QObject::trUtf8("No log file specified."));
            }
        }

        i++;
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

#ifdef Q_OS_UNIX
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

#endif

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
