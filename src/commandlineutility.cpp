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
#include "common.h"
#include "eventhandlerfactory.h"

#include <QDebug>
#include <QStringListIterator>
#include <QFileInfo>
#include <QTextStream>

#ifdef Q_OS_UNIX
#include <QApplication>
#endif



QRegExp CommandLineUtility::trayRegexp = QRegExp("--tray");
QRegExp CommandLineUtility::helpRegexp = QRegExp("(-h|--help)");
QRegExp CommandLineUtility::versionRegexp = QRegExp("(-v|--version)");
QRegExp CommandLineUtility::noTrayRegexp = QRegExp("--no-tray");
QRegExp CommandLineUtility::loadProfileRegexp = QRegExp("--profile");
QRegExp CommandLineUtility::loadProfileForControllerRegexp = QRegExp("--profile-controller");
QRegExp CommandLineUtility::hiddenRegexp = QRegExp("--hidden");
QRegExp CommandLineUtility::unloadRegexp = QRegExp("--unload");
QRegExp CommandLineUtility::startSetRegexp = QRegExp("--startSet");
QRegExp CommandLineUtility::gamepadListRegexp = QRegExp("(-l|--list)");
QRegExp CommandLineUtility::mappingRegexp = QRegExp("--map");
QRegExp CommandLineUtility::qtStyleRegexp = QRegExp("-style");
QRegExp CommandLineUtility::logLevelRegexp = QRegExp("--log-level");
QRegExp CommandLineUtility::logFileRegexp = QRegExp("--log-file");
QRegExp CommandLineUtility::eventgenRegexp = QRegExp("--eventgen");
QRegExp CommandLineUtility::nextRegexp = QRegExp("--next");

#ifdef Q_OS_UNIX
QRegExp CommandLineUtility::daemonRegexp = QRegExp("--daemon|-d");

  #ifdef WITH_X11
    QRegExp CommandLineUtility::displayRegexp = QRegExp("--display");
  #endif
#endif

QStringList CommandLineUtility::eventGeneratorsList = EventHandlerFactory::buildEventGeneratorList();

CommandLineUtility::CommandLineUtility(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    launchInTray = false;
    helpRequest = false;
    versionRequest = false;
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

void CommandLineUtility::parseArguments(QStringList &arguments)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QStringListIterator iter(arguments);
    while (iter.hasNext() && !encounteredError)
    {
        QString temp = iter.next();
        if (helpRegexp.exactMatch(temp))
        {
            helpRequest = true;
        }
        else if (versionRegexp.exactMatch(temp))
        {
            versionRequest = true;
        }
        else if (trayRegexp.exactMatch(temp))
        {
            launchInTray = true;
            hideTrayIcon = false;
        }
        else if (noTrayRegexp.exactMatch(temp))
        {
            hideTrayIcon = true;
            launchInTray = false;
        }
        else if (loadProfileRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                temp = iter.next();
                QFileInfo fileInfo(temp);
                if (fileInfo.exists())
                {
                    if ((fileInfo.suffix() != "amgp") && (fileInfo.suffix() != "xml"))
                    {
                        setErrorMessage(trUtf8("Profile location %1 is not an XML file.").arg(temp));
                    }
                    else
                    {
                        QString tempProfileLocation = fileInfo.absoluteFilePath();
                        ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                        tempInfo.setProfileLocation(tempProfileLocation);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                }
                else
                {
                    setErrorMessage(trUtf8("Profile location %1 does not exist.").arg(temp));
                }
            }
        }
        else if (loadProfileForControllerRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                temp = iter.next();

                bool validNumber = false;
                int tempNumber = temp.toInt(&validNumber);
                if (validNumber)
                {
                    if (controllerNumber == 0)
                    {
                        controllerNumber = tempNumber;
                    }

                    ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                    tempInfo.setControllerNumber(tempNumber);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else if (!temp.isEmpty())
                {
                    if (controllerIDString.isEmpty())
                    {
                        controllerIDString = temp;
                    }

                    ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                    tempInfo.setControllerID(temp);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else
                {
                    setErrorMessage(trUtf8("Controller identifier is not a valid value."));
                }
            }
        }
        else if (hiddenRegexp.exactMatch(temp))
        {
            hiddenRequest = true;
        }
        else if (unloadRegexp.exactMatch(temp))
        {
            ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
            tempInfo.setProfileLocation("");
            tempInfo.setUnloadRequest(true);
            controllerOptionsList.replace(currentListsIndex, tempInfo);

            if (iter.hasNext())
            {
                temp = iter.next();

                if (!isPossibleCommand(temp))
                {
                    // A value has been passed. Attempt
                    // to validate the value.
                    bool validNumber = false;
                    int tempNumber = temp.toInt(&validNumber);
                    if (validNumber)
                    {
                        controllerNumber = tempNumber;
                        tempInfo = controllerOptionsList.at(currentListsIndex);
                        tempInfo.setControllerNumber(controllerNumber);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                    else if (!temp.isEmpty())
                    {
                        controllerIDString = temp;
                        tempInfo = controllerOptionsList.at(currentListsIndex);
                        tempInfo.setControllerID(controllerIDString);
                        controllerOptionsList.replace(currentListsIndex, tempInfo);
                    }
                    else
                    {
                        setErrorMessage(trUtf8("Controller identifier is not a valid value."));
                    }
                }
                else
                {
                    // Grabbed a possible command-line option.
                    // Move iterator back to previous item.
                    iter.previous();
                }
            }
            else
            {
                unloadProfile = true;
                profileLocation = "";
            }
        }
        else if (startSetRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                temp = iter.next();

                bool validNumber = false;
                int tempNumber = temp.toInt(&validNumber);
                if (validNumber && (tempNumber >= 1) && (tempNumber <= 8))
                {
                    startSetNumber = tempNumber;
                    ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                    tempInfo.setStartSetNumber(startSetNumber);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else if (validNumber)
                {
                    setErrorMessage(trUtf8("An invalid set number '%1' was specified.").arg(tempNumber));
                }

                if (iter.hasNext())
                {
                    temp = iter.next();

                    if (!isPossibleCommand(temp))
                    {
                        if (validNumber)
                        {
                            controllerNumber = tempNumber;
                            ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                            tempInfo.setControllerNumber(controllerNumber);
                            controllerOptionsList.replace(currentListsIndex, tempInfo);
                        }
                        else if (!temp.isEmpty())
                        {
                            controllerIDString = temp;
                            ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                            tempInfo.setControllerID(controllerIDString);
                            controllerOptionsList.replace(currentListsIndex, tempInfo);
                        }
                        else
                        {
                            setErrorMessage(trUtf8("Controller identifier '%s'' is not a valid value.").arg(temp));
                        }
                    }
                    else
                    {
                        // Grabbed a possible command-line option.
                        // Move iterator back to previous item.
                        iter.previous();
                    }
                }
            }
            else
            {
                setErrorMessage(trUtf8("No set number was specified."));
            }
        }
        else if (nextRegexp.exactMatch(temp))
        {
            currentListsIndex++;

            ControllerOptionsInfo tempInfo;
            controllerOptionsList.append(tempInfo);
        }

#ifdef USE_SDL_2
        else if (gamepadListRegexp.exactMatch(temp))
        {
            listControllers = true;
        }
        else if (mappingRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                temp = iter.next();

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
                    setErrorMessage(trUtf8("Controller identifier is not a valid value."));
                }
            }
            else
            {
                setErrorMessage(trUtf8("No controller was specified."));
            }
        }
#endif

#ifdef Q_OS_UNIX
        else if (daemonRegexp.exactMatch(temp))
        {
            daemonMode = true;
        }
  #ifdef WITH_X11
        else if (displayRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                displayString = iter.next();
            }
            else
            {
                setErrorMessage(trUtf8("No display string was specified."));
                //errorsteam << trUtf8("No display string was specified.") << endl;
                //encounteredError = true;
            }
        }
  #endif
#endif

#if (defined (Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)) \
     || (defined(Q_OS_WIN) && defined(WITH_VMULTI))
        else if (eventgenRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                QString temp = iter.next();

                if (!eventGeneratorsList.contains(temp))
                {
                    eventGenerator = "";
                    setErrorMessage(trUtf8("An invalid event generator was specified."));
                    //errorsteam << trUtf8("An invalid event generator was specified.") << endl;
                    //encounteredError = true;
                }
                else
                {
                    eventGenerator = temp;
                }
            }
            else
            {
                setErrorMessage(trUtf8("No event generator string was specified."));
                //errorsteam << trUtf8("No event generator string was specified.") << endl;
                //encounteredError = true;
            }
        }
#endif

        else if (qtStyleRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                // Skip over argument
                iter.next();
            }
            else
            {
                setErrorMessage(trUtf8("Qt style flag was detected but no style was specified."));
                //errorsteam << trUtf8("Qt style flag was detected but no style was specified.") << endl;
                //encounteredError = true;
            }
        }
        else if (logLevelRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
                QString temp = iter.next();
                if (temp == "debug")
                {
                    currentLogLevel = Logger::LOG_DEBUG;
                }
                else if (temp == "info")
                {
                    currentLogLevel = Logger::LOG_INFO;
                }
                /*else if (temp == "warn")
                {
                    currentLogLevel = Logger::LOG_WARNING;
                }
                else if (temp == "error")
                {
                    currentLogLevel = Logger::LOG_ERROR;
                }
                */
            }
	}
        else if (logFileRegexp.exactMatch(temp))
        {
            if (iter.hasNext())
            {
	      currentLogFile = iter.next();
            }
            else
            {
                setErrorMessage(trUtf8("No log file specified."));
                //errorsteam << trUtf8("No log level specified.") << endl;
                //encounteredError = true;
            }

        }
        else if (isPossibleCommand(temp))
        {
            // Flag is unrecognized. Assume that it is a Qt option.
            if (iter.hasNext())
            {
                // Check next argument
                QString nextarg = iter.next();
                if (isPossibleCommand(nextarg))
                {
                    // Flag likely didn't take an argument. Move iterator
                    // back.
                    iter.previous();
                }
            }
        }
        // Check if this is the last argument. If it is and no command line flag
        // is active, the final argument is likely a profile that has
        // been specified.
        else if (!temp.isEmpty() && !iter.hasNext())
        {
            // If the file exists and it is an xml file, assume that it is a
            // profile.
            QFileInfo fileInfo(temp);
            if (fileInfo.exists())
            {
                if ((fileInfo.suffix() != "amgp") && (fileInfo.suffix() != "xml"))
                {
                    setErrorMessage(trUtf8("Profile location %1 is not an XML file.").arg(temp));
                }
                else
                {
                    profileLocation = fileInfo.absoluteFilePath();
                    // Only use properties if no other OptionsInfo lines have
                    // been defined.
                    if (currentListsIndex > 0)
                    {
                        controllerNumber = 0;
                        controllerIDString.clear();
                    }
                }
            }
            else
            {
                setErrorMessage(trUtf8("Profile location %1 does not exist.").arg(temp));
            }
        }
    }
}

bool CommandLineUtility::isLaunchInTrayEnabled()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return launchInTray;
}

void CommandLineUtility::printHelp()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QTextStream out(stdout);
    out << trUtf8("antimicro version") << " " << PadderCommon::programVersion << endl;
    out << trUtf8("Usage: antimicro [options...] [profile]") << endl;
    out << endl;
    out << trUtf8("Options") << ":" << endl;
    out << "-h, --help                    " << " " << trUtf8("Print help text.") << endl;
    out << "-v, --version                 " << " " << trUtf8("Print version information.") << endl;
    out << "--tray                        " << " " << trUtf8("Launch program in system tray only.") << endl;
    out << "--no-tray                     " << " " << trUtf8("Launch program with the tray menu disabled.") << endl;
    out << "--hidden                      " << " " << trUtf8("Launch program without the main window\n                               displayed.") << endl;
    out << "--profile <location>          " << " " <<
           trUtf8("Launch program with the configuration file\n                               selected as the default for selected\n                               controllers. Defaults to all controllers.")
        << endl;
    out << "--profile-controller <value>  " << " "
        << trUtf8("Apply configuration file to a specific\n                               controller. Value can be a\n                               controller index, name, or GUID.")
        << endl;
    out << "--unload [<value>]            " << " " << trUtf8("Unload currently enabled profile(s). \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--startSet <number> [<value>] " << " " << trUtf8("Start joysticks on a specific set.   \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--next                        " << " " << trUtf8("Advance profile loading set options.")
        << endl;

#ifdef Q_OS_UNIX
    out << "-d, --daemon                  " << " "
        << trUtf8("Launch program as a daemon.") << endl;
    out << "--log-level {debug,info}      " << " " << trUtf8("Enable logging.") << endl;

    #ifdef WITH_X11
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    out << "--display <value>             " << " "
        << trUtf8("Use specified display for X11 calls.\n"
              "                               Useful for ssh.")
        << endl;
    }
    #endif
#endif

#if defined(Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)
    out << "--eventgen {xtest,uinput}     " << " "
        << trUtf8("Choose between using XTest support and uinput\n"
           "                               support for event generation. Default: xtest.")
        << endl;
#elif defined(Q_OS_WIN) && defined(WITH_VMULTI)
    out << "--eventgen {sendinput,vmulti} " << " "
        << trUtf8("Choose between using SendInput and vmulti\n"
           "                           support for event generation. Default: sendinput.")
        << endl;
#endif

#ifdef USE_SDL_2
    out << "-l, --list                    " << " "
        << trUtf8("Print information about joysticks detected by \n"
              "                               SDL.") << endl;
    out << "--map <value>                 " << " "
        << trUtf8("Open game controller mapping window of selected\n"
              "                               controller. Value can be a controller index or\n"
              "                               GUID.")
        << endl;
#endif

}

QString CommandLineUtility::generateHelpString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    QTextStream out(&temp);
    out << trUtf8("antimicro version") << " " << PadderCommon::programVersion << endl;
    out << trUtf8("Usage: antimicro [options...] [profile]") << endl;
    out << endl;
    out << trUtf8("Options") << ":" << endl;
    out << "-h, --help                    " << " " << trUtf8("Print help text.") << endl;
    out << "-v, --version                 " << " " << trUtf8("Print version information.") << endl;
    out << "--tray                        " << " " << trUtf8("Launch program in system tray only.") << endl;
    out << "--no-tray                     " << " " << trUtf8("Launch program with the tray menu disabled.") << endl;
    out << "--hidden                      " << " " << trUtf8("Launch program without the main window\n                               displayed.") << endl;
    out << "--profile <location>          " << " " <<
           trUtf8("Launch program with the configuration file\n                               selected as the default for selected\n                               controllers. Defaults to all controllers.")
        << endl;
    out << "--profile-controller <value>  " << " "
        << trUtf8("Apply configuration file to a specific\n                               controller. Value can be a\n                               controller index, name, or GUID.")
        << endl;
    out << "--unload [<value>]            " << " " << trUtf8("Unload currently enabled profile(s). \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--startSet <number> [<value>] " << " " << trUtf8("Start joysticks on a specific set.   \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--next                        " << " " << trUtf8("Advance profile loading set options.")
        << endl;

#ifdef Q_OS_UNIX
    out << "-d, --daemon                  " << " "
        << trUtf8("Launch program as a daemon.") << endl;
    out << "--log-level {debug,info}      " << " " << trUtf8("Enable logging.") << endl;
    #ifdef WITH_X11
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    out << "--display <value>             " << " "
        << trUtf8("Use specified display for X11 calls.\n"
              "                               Useful for ssh.")
        << endl;
    }
    #endif
#endif

#if defined(Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)
    out << "--eventgen {xtest,uinput}     " << " "
        << trUtf8("Choose between using XTest support and uinput\n"
           "                               support for event generation. Default: xtest.")
        << endl;
#elif defined(Q_OS_WIN) && defined(WITH_VMULTI)
    out << "--eventgen {sendinput,vmulti} " << " "
        << trUtf8("Choose between using SendInput and vmulti\n"
           "                           support for event generation. Default: sendinput.")
        << endl;
#endif

#ifdef USE_SDL_2
    out << "-l, --list                    " << " "
        << trUtf8("Print information about joysticks detected by \n"
              "                               SDL.") << endl;
    out << "--map <value>                 " << " "
        << trUtf8("Open game controller mapping window of selected\n"
              "                               controller. Value can be a controller index or\n"
              "                               GUID.")
        << endl;
#endif

    return temp;
}

bool CommandLineUtility::isHelpRequested()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return helpRequest;
}

bool CommandLineUtility::isVersionRequested()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return versionRequest;
}

void CommandLineUtility::printVersionString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QTextStream out(stdout);
    out << trUtf8("antimicro version") << " " << PadderCommon::programVersion << endl;
}

QString CommandLineUtility::generateVersionString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    QTextStream out(&temp);
    out << trUtf8("antimicro version") << " " << PadderCommon::programVersion;

    return temp;
}

bool CommandLineUtility::isTrayHidden()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return hideTrayIcon;
}

bool CommandLineUtility::hasProfile()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return !profileLocation.isEmpty();
}

bool CommandLineUtility::hasControllerNumber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return (controllerNumber > 0);
}

QString CommandLineUtility::getProfileLocation()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return profileLocation;
}

int CommandLineUtility::getControllerNumber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return controllerNumber;
}

bool CommandLineUtility::hasError()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return encounteredError;
}

bool CommandLineUtility::isHiddenRequested()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return hiddenRequest;
}

bool CommandLineUtility::hasControllerID()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return !controllerIDString.isEmpty();
}

QString CommandLineUtility::getControllerID()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return controllerIDString;
}

bool CommandLineUtility::isUnloadRequested()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return unloadProfile;
}

int CommandLineUtility::getStartSetNumber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return startSetNumber;
}

int CommandLineUtility::getJoyStartSetNumber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return startSetNumber - 1;
}

bool CommandLineUtility::isPossibleCommand(QString temp)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = false;

    if (temp.startsWith("--") || temp.startsWith("-"))
    {
        result = true;
    }

    return result;
}

bool CommandLineUtility::shouldListControllers()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return listControllers;
}

bool CommandLineUtility::shouldMapController()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return mappingController;
}

QString CommandLineUtility::getEventGenerator()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return eventGenerator;
}

#ifdef Q_OS_UNIX
bool CommandLineUtility::launchAsDaemon()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return daemonMode;
}

QString CommandLineUtility::getDisplayString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return displayString;
}

#endif

Logger::LogLevel CommandLineUtility::getCurrentLogLevel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return currentLogLevel;
}

QString CommandLineUtility::getCurrentLogFile() {
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

  return currentLogFile;
}

QString CommandLineUtility::getErrorText() {

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return errorText;
}

void CommandLineUtility::setErrorMessage(QString temp)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    errorText = temp;
    encounteredError = true;
}

QList<ControllerOptionsInfo>* CommandLineUtility::getControllerOptionsList()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return &controllerOptionsList;
}

bool CommandLineUtility::hasProfileInOptions()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = false;

    QListIterator<ControllerOptionsInfo> iter(controllerOptionsList);
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
