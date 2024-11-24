/* antimicrox Gamepad to KB+M event mapper
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

#include "common.h"
#include "eventhandlerfactory.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>

#include <stdexcept>

QStringList CommandLineUtility::eventGeneratorsList = EventHandlerFactory::buildEventGeneratorList();

CommandLineUtility::CommandLineUtility(QObject *parent)
    : QObject(parent)
{
    launchInTray = false;
    hideTrayIcon = false;
    profileLocation = "";
    controllerNumber = 0;
    hiddenRequest = false;
    showRequest = false;
    unloadProfile = false;
    startSetNumber = 0;
    listControllers = false;
    currentLogLevel = Logger::LOG_NONE;

    currentListsIndex = 0;
    ControllerOptionsInfo tempInfo;
    controllerOptionsList.append(tempInfo);

    eventGenerator = EventHandlerFactory::fallBackIdentifier();
}

void CommandLineUtility::parseArguments(const QApplication &parsed_app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("antimicrox", "Graphical program used to map keyboard buttons and mouse controls to "
                                                  "a "
                                                  "gamepad. Useful for playing games with no gamepad support."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        // A boolean option with a single name (-p)
        {"tray", QCoreApplication::translate("main", "Launch program in system tray only.")},
        // A boolean option with multiple names (-f, --force)
        {"no-tray", QCoreApplication::translate("main", "Launch program with the tray menu disabled")},
        // An option with a value
        {"hidden", QCoreApplication::translate("main", "Launch program without the main window displayed")},
        {"show", QCoreApplication::translate(
                     "main", "Show app window when hidden. (Used for unhiding window of already running app instance).")},
        {"profile",
         QCoreApplication::translate("main",
                                     "Launch program with the configuration file selected as the default for selected "
                                     "controllers, or change active profile of running app. Defaults to all controllers"),
         QCoreApplication::translate("main", "location")},
        {"profile-controller",
         QCoreApplication::translate("main", "Apply configuration file to a specific controller. Value "
                                             "can be a controller index, name, or GUID"),
         QCoreApplication::translate("main", "value")},
        {"unload", QCoreApplication::translate("main", "Unload currently enabled profile(s)"),
         QCoreApplication::translate("main", "value(s)")},
        {"startSet",
         QCoreApplication::translate("main", "Start joysticks on a specific set. Value can be a "
                                             "controller index, name, or GUID"),
         QCoreApplication::translate("main", "number value")},
        {"log-level",
         QCoreApplication::translate("main", "Enable logging. Levels (from the least strict): warn,info,verbose,debug"),
         QCoreApplication::translate("main", "log-type")},
        {"log-file", QCoreApplication::translate("main", "Choose a file for logs writing"),
         QCoreApplication::translate("main", "filename")},
        {"eventgen",
         QCoreApplication::translate("main", "Choose between using XTest support and uinput support "
                                             "for event generation. Use only if you have "
                                             "enabled xtest and uinput options on Linux or vmulti on "
                                             "Windows. Default: xtest."),
         QCoreApplication::translate("main", "event-generation-type"), "xtest"}, // default
        {{"list", "l"},
         QCoreApplication::translate("main", "Print information about joysticks detected by SDL. Use "
                                             "only if you have sdl "
                                             "library. You can check your controller index, name or "
                                             "even GUID.")},
        {"next", QCoreApplication::translate("main", "Load multiple profiles for different controllers. This option is "
                                                     "meant to be used with profile-controller and profile options.")},

    });

    parser.process(parsed_app);

    int i = 0;

    while ((i < parser.optionNames().count()))
    {
        if (parser.isSet("tray"))
        {
            qDebug() << "tray is set";

            launchInTray = true;
            hideTrayIcon = false;
        } else if (parser.isSet("no-tray"))
        {
            qDebug() << "no-tray is set";

            hideTrayIcon = true;
            launchInTray = false;
        }

        if (parser.isSet("profile"))
        {
            parseArgsProfile(parser);
        }

        if (parser.isSet("profile-controller"))
        {
            parseArgsPrControle(parser);
        }

        if (parser.isSet("hidden"))
        {
            hiddenRequest = true;
        }

        if (parser.isSet("show"))
        {
            showRequest = true;
        }

        if (parser.isSet("unload"))
        {
            parseArgsUnload(parser);
        }

        if (parser.isSet("startSet"))
        {
            parseArgsStartSet(parser);
        }

        if (parser.isSet("next"))
        {
            currentListsIndex++;
            ControllerOptionsInfo tempInfo;
            controllerOptionsList.append(tempInfo);
        }

        if (parser.isSet("list"))
        {
            listControllers = true;
        }

#if (defined(WITH_UINPUT) && defined(WITH_XTEST))

        if (parser.isSet("eventgen"))
        {
            QString eventGenText = parser.value("eventgen");

            if (!eventGenText.isEmpty() && !eventGeneratorsList.contains(eventGenText))
            {
                eventGenerator = "";
                throw std::runtime_error(QObject::tr("An invalid event generator was specified.").toStdString());
            } else if (!eventGenText.isEmpty() && eventGeneratorsList.contains(eventGenText))
            {
                eventGenerator = eventGenText;
            } else
            {
                throw std::runtime_error(QObject::tr("No event generator string was specified.").toStdString());
            }
        }
#endif

        if (parser.isSet("log-level"))
        {
            QString logLevelText = parser.value("log-level");

            if (logLevelText == "debug")
                currentLogLevel = Logger::LOG_DEBUG;
            else if (logLevelText == "verbose")
                currentLogLevel = Logger::LOG_VERBOSE;
            else if (logLevelText == "info")
                currentLogLevel = Logger::LOG_INFO;
            else if (logLevelText == "warn")
                currentLogLevel = Logger::LOG_WARNING;
            else if (logLevelText == "error")
                currentLogLevel = Logger::LOG_ERROR;
            else
            {
                throw std::runtime_error((QObject::tr("Unknown log level: ") + logLevelText).toStdString());
            }
        }

        if (parser.isSet("log-file"))
        {
            if (!parser.value("log-file").isEmpty())
            {
                currentLogFile = parser.value("log-file");
            } else
            {
                throw std::runtime_error(QObject::tr("No log file specified.").toStdString());
            }
        }

        i++;
    }
    if (showRequest && hiddenRequest)
        throw std::runtime_error(QObject::tr("Specified contradicting flags: --show and --hidden").toStdString());
}

void CommandLineUtility::parseArgsProfile(const QCommandLineParser &parser)
{
    QFileInfo profileFileInfo(parser.value("profile"));

    if (profileFileInfo.exists() && (profileFileInfo.suffix() != "amgp") && (profileFileInfo.suffix() != "xml"))
    {
        throw std::runtime_error(
            QObject::tr("Profile location %1 is not an XML file.").arg(parser.value("profile")).toStdString());
    } else if (profileFileInfo.exists() && ((profileFileInfo.suffix() == "amgp") || (profileFileInfo.suffix() == "xml")))
    {
        QString tempProfileLocation = profileFileInfo.absoluteFilePath();
        qDebug() << "Profile file is set: " << tempProfileLocation;
        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setProfileLocation(tempProfileLocation);
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    } else
    {
        throw std::runtime_error(
            QObject::tr("Profile location %1 does not exist.").arg(parser.value("profile")).toStdString());
    }
}

void CommandLineUtility::parseArgsPrControle(const QCommandLineParser &parser)
{
    bool validNumber = false;
    int tempNumber = parser.value("profile-controller").toInt(&validNumber);

    if (validNumber)
    {
        if (controllerNumber == 0)
            controllerNumber = tempNumber;

        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setControllerNumber(tempNumber);
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    } else if (!parser.value("profile-controller").isEmpty())
    {
        if (controllerIDString.isEmpty())
            controllerIDString = parser.value("profile-controller");

        ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
        tempInfo.setControllerID(parser.value("profile-controller"));
        controllerOptionsList.replace(currentListsIndex, tempInfo);
    } else
    {
        throw std::runtime_error(QObject::tr("Controller identifier is not a valid value.").toStdString());
    }
}

void CommandLineUtility::parseArgsUnload(const QCommandLineParser &parser)
{
    ControllerOptionsInfo gameControllerOptionInfo = getControllerOptionsList().at(currentListsIndex);
    gameControllerOptionInfo.setProfileLocation("");
    gameControllerOptionInfo.setUnloadRequest(true);
    controllerOptionsList.replace(currentListsIndex, gameControllerOptionInfo);

    QStringList parserVals = QStringList() << parser.values("unload");
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
        } else if (!unloadedProfText.isEmpty())
        {
            controllerIDString = unloadedProfText;
            gameControllerOptionInfo = getControllerOptionsList().at(currentListsIndex);
            gameControllerOptionInfo.setControllerID(controllerIDString);
            controllerOptionsList.replace(currentListsIndex, gameControllerOptionInfo);
        } else
        {
            throw std::runtime_error(QObject::tr("Controller identifier is not a valid value.").toStdString());
        }
    } else
    {
        unloadProfile = true;
        profileLocation = "";
    }
}

void CommandLineUtility::parseArgsStartSet(const QCommandLineParser &parser)
{
    QStringList parserValues = QStringList() << parser.values("startSet");
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
        } else if (validNumber)
        {
            throw std::runtime_error(QObject::tr("An invalid set number '%1' was specified.").arg(tempNumber).toStdString());
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
            } else if (!startSetText.isEmpty())
            {
                controllerIDString = startSetText;
                ControllerOptionsInfo tempInfo = getControllerOptionsList().at(currentListsIndex);
                tempInfo.setControllerID(controllerIDString);
                controllerOptionsList.replace(currentListsIndex, tempInfo);
            } else
            {
                throw std::runtime_error(
                    QObject::tr("Controller identifier '%s'' is not a valid value.").arg(startSetText).toStdString());
            }
        }
    } else
    {
        throw std::runtime_error(QObject::tr("No set number was specified.").toStdString());
    }
}

bool CommandLineUtility::isLaunchInTrayEnabled() { return launchInTray; }

bool CommandLineUtility::isTrayHidden() { return hideTrayIcon; }

bool CommandLineUtility::hasProfile() { return !profileLocation.isEmpty(); }

bool CommandLineUtility::hasControllerNumber() { return (controllerNumber > 0); }

QString CommandLineUtility::getProfileLocation() { return profileLocation; }

int CommandLineUtility::getControllerNumber() { return controllerNumber; }

bool CommandLineUtility::isHiddenRequested() { return hiddenRequest; }

bool CommandLineUtility::isShowRequested() { return showRequest; }

bool CommandLineUtility::hasControllerID() { return !controllerIDString.isEmpty(); }

QString CommandLineUtility::getControllerID() { return controllerIDString; }

bool CommandLineUtility::isUnloadRequested() { return unloadProfile; }

int CommandLineUtility::getStartSetNumber() { return startSetNumber; }

int CommandLineUtility::getJoyStartSetNumber() { return startSetNumber - 1; }

bool CommandLineUtility::shouldListControllers() { return listControllers; }

QString CommandLineUtility::getEventGenerator() { return eventGenerator; }

Logger::LogLevel CommandLineUtility::getCurrentLogLevel() { return currentLogLevel; }

QString CommandLineUtility::getCurrentLogFile() { return currentLogFile; }

QList<ControllerOptionsInfo> const &CommandLineUtility::getControllerOptionsList() { return controllerOptionsList; }

bool CommandLineUtility::hasProfileInOptions()
{
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
