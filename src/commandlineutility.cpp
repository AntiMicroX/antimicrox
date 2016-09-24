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

//#include <QDebug>
#include <QStringListIterator>
#include <QFileInfo>
#include <QTextStream>

#ifdef Q_OS_UNIX
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif
#endif

#include "commandlineutility.h"
#include "common.h"

#include "eventhandlerfactory.h"

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
                    if (fileInfo.suffix() != "amgp" && fileInfo.suffix() != "xml")
                    {
                        setErrorMessage(tr("Profile location %1 is not an XML file.").arg(temp));
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
                    setErrorMessage(tr("Profile location %1 does not exist.").arg(temp));
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
                    setErrorMessage(tr("Controller identifier is not a valid value."));
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
                        setErrorMessage(tr("Controller identifier is not a valid value."));
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
                if (validNumber && tempNumber >= 1 && tempNumber <= 8)
                {
                    startSetNumber = tempNumber;
                    ControllerOptionsInfo tempInfo = controllerOptionsList.at(currentListsIndex);
                    tempInfo.setStartSetNumber(startSetNumber);
                    controllerOptionsList.replace(currentListsIndex, tempInfo);
                }
                else if (validNumber)
                {
                    setErrorMessage(tr("An invalid set number '%1' was specified.").arg(tempNumber));
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
                            setErrorMessage(tr("Controller identifier '%s'' is not a valid value.").arg(temp));
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
                setErrorMessage(tr("No set number was specified."));
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
                    setErrorMessage(tr("Controller identifier is not a valid value."));
                }
            }
            else
            {
                setErrorMessage(tr("No controller was specified."));
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
                setErrorMessage(tr("No display string was specified."));
                //errorsteam << tr("No display string was specified.") << endl;
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
                    setErrorMessage(tr("An invalid event generator was specified."));
                    //errorsteam << tr("An invalid event generator was specified.") << endl;
                    //encounteredError = true;
                }
                else
                {
                    eventGenerator = temp;
                }
            }
            else
            {
                setErrorMessage(tr("No event generator string was specified."));
                //errorsteam << tr("No event generator string was specified.") << endl;
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
                setErrorMessage(tr("Qt style flag was detected but no style was specified."));
                //errorsteam << tr("Qt style flag was detected but no style was specified.") << endl;
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
                setErrorMessage(tr("No log file specified."));
                //errorsteam << tr("No log level specified.") << endl;
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
                if (fileInfo.suffix() != "amgp" && fileInfo.suffix() != "xml")
                {
                    setErrorMessage(tr("Profile location %1 is not an XML file.").arg(temp));
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
                setErrorMessage(tr("Profile location %1 does not exist.").arg(temp));
            }
        }
    }
}

bool CommandLineUtility::isLaunchInTrayEnabled()
{
    return launchInTray;
}

void CommandLineUtility::printHelp()
{
    QTextStream out(stdout);
    out << tr("antimicro version") << " " << PadderCommon::programVersion << endl;
    out << tr("Usage: antimicro [options...] [profile]") << endl;
    out << endl;
    out << tr("Options") << ":" << endl;
    out << "-h, --help                    " << " " << tr("Print help text.") << endl;
    out << "-v, --version                 " << " " << tr("Print version information.") << endl;
    out << "--tray                        " << " " << tr("Launch program in system tray only.") << endl;
    out << "--no-tray                     " << " " << tr("Launch program with the tray menu disabled.") << endl;
    out << "--hidden                      " << " " << tr("Launch program without the main window\n                               displayed.") << endl;
    out << "--profile <location>          " << " " <<
           tr("Launch program with the configuration file\n                               selected as the default for selected\n                               controllers. Defaults to all controllers.")
        << endl;
    out << "--profile-controller <value>  " << " "
        << tr("Apply configuration file to a specific\n                               controller. Value can be a\n                               controller index, name, or GUID.")
        << endl;
    out << "--unload [<value>]            " << " " << tr("Unload currently enabled profile(s). \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--startSet <number> [<value>] " << " " << tr("Start joysticks on a specific set.   \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--next                        " << " " << tr("Advance profile loading set options.")
        << endl;

#ifdef Q_OS_UNIX
    out << "-d, --daemon                  " << " "
        << tr("Launch program as a daemon.") << endl;
    out << "--log-level {debug,info}      " << " " << tr("Enable logging.") << endl;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    out << "--display <value>             " << " "
        << tr("Use specified display for X11 calls.\n"
              "                               Useful for ssh.")
        << endl;
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #endif
#endif

#if defined(Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)
    out << "--eventgen {xtest,uinput}     " << " "
        << tr("Choose between using XTest support and uinput\n"
           "                               support for event generation. Default: xtest.")
        << endl;
#elif defined(Q_OS_WIN) && defined(WITH_VMULTI)
    out << "--eventgen {sendinput,vmulti} " << " "
        << tr("Choose between using SendInput and vmulti\n"
           "                           support for event generation. Default: sendinput.")
        << endl;
#endif

#ifdef USE_SDL_2
    out << "-l, --list                    " << " "
        << tr("Print information about joysticks detected by \n"
              "                               SDL.") << endl;
    out << "--map <value>                 " << " "
        << tr("Open game controller mapping window of selected\n"
              "                               controller. Value can be a controller index or\n"
              "                               GUID.")
        << endl;
#endif

}

QString CommandLineUtility::generateHelpString()
{
    QString temp;
    QTextStream out(&temp);
    out << tr("antimicro version") << " " << PadderCommon::programVersion << endl;
    out << tr("Usage: antimicro [options...] [profile]") << endl;
    out << endl;
    out << tr("Options") << ":" << endl;
    out << "-h, --help                    " << " " << tr("Print help text.") << endl;
    out << "-v, --version                 " << " " << tr("Print version information.") << endl;
    out << "--tray                        " << " " << tr("Launch program in system tray only.") << endl;
    out << "--no-tray                     " << " " << tr("Launch program with the tray menu disabled.") << endl;
    out << "--hidden                      " << " " << tr("Launch program without the main window\n                               displayed.") << endl;
    out << "--profile <location>          " << " " <<
           tr("Launch program with the configuration file\n                               selected as the default for selected\n                               controllers. Defaults to all controllers.")
        << endl;
    out << "--profile-controller <value>  " << " "
        << tr("Apply configuration file to a specific\n                               controller. Value can be a\n                               controller index, name, or GUID.")
        << endl;
    out << "--unload [<value>]            " << " " << tr("Unload currently enabled profile(s). \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--startSet <number> [<value>] " << " " << tr("Start joysticks on a specific set.   \n                               Value can be a controller index, name, or GUID.")
        << endl;
    out << "--next                        " << " " << tr("Advance profile loading set options.")
        << endl;

#ifdef Q_OS_UNIX
    out << "-d, --daemon                  " << " "
        << tr("Launch program as a daemon.") << endl;
    out << "--log-level {debug,info}      " << " " << tr("Enable logging.") << endl;
    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    out << "--display <value>             " << " "
        << tr("Use specified display for X11 calls.\n"
              "                               Useful for ssh.")
        << endl;
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #endif
#endif

#if defined(Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)
    out << "--eventgen {xtest,uinput}     " << " "
        << tr("Choose between using XTest support and uinput\n"
           "                               support for event generation. Default: xtest.")
        << endl;
#elif defined(Q_OS_WIN) && defined(WITH_VMULTI)
    out << "--eventgen {sendinput,vmulti} " << " "
        << tr("Choose between using SendInput and vmulti\n"
           "                           support for event generation. Default: sendinput.")
        << endl;
#endif

#ifdef USE_SDL_2
    out << "-l, --list                    " << " "
        << tr("Print information about joysticks detected by \n"
              "                               SDL.") << endl;
    out << "--map <value>                 " << " "
        << tr("Open game controller mapping window of selected\n"
              "                               controller. Value can be a controller index or\n"
              "                               GUID.")
        << endl;
#endif

    return temp;
}

bool CommandLineUtility::isHelpRequested()
{
    return helpRequest;
}

bool CommandLineUtility::isVersionRequested()
{
    return versionRequest;
}

void CommandLineUtility::printVersionString()
{
    QTextStream out(stdout);
    out << tr("antimicro version") << " " << PadderCommon::programVersion << endl;
}

QString CommandLineUtility::generateVersionString()
{
    QString temp;
    QTextStream out(&temp);
    out << tr("antimicro version") << " " << PadderCommon::programVersion;

    return temp;
}

bool CommandLineUtility::isTrayHidden()
{
    return hideTrayIcon;
}

bool CommandLineUtility::hasProfile()
{
    return !profileLocation.isEmpty();
}

bool CommandLineUtility::hasControllerNumber()
{
    return (controllerNumber > 0);
}

QString CommandLineUtility::getProfileLocation()
{
    return profileLocation;
}

unsigned int CommandLineUtility::getControllerNumber()
{
    return controllerNumber;
}

bool CommandLineUtility::hasError()
{
    return encounteredError;
}

bool CommandLineUtility::isHiddenRequested()
{
    return hiddenRequest;
}

bool CommandLineUtility::hasControllerID()
{
    return !controllerIDString.isEmpty();
}

QString CommandLineUtility::getControllerID()
{
    return controllerIDString;
}

bool CommandLineUtility::isUnloadRequested()
{
    return unloadProfile;
}

unsigned int CommandLineUtility::getStartSetNumber()
{
    return startSetNumber;
}

unsigned int CommandLineUtility::getJoyStartSetNumber()
{
    return startSetNumber - 1;
}

bool CommandLineUtility::isPossibleCommand(QString temp)
{
    bool result = false;

    if (temp.startsWith("--") || temp.startsWith("-"))
    {
        result = true;
    }

    return result;
}

bool CommandLineUtility::shouldListControllers()
{
    return listControllers;
}

bool CommandLineUtility::shouldMapController()
{
    return mappingController;
}

QString CommandLineUtility::getEventGenerator()
{
    return eventGenerator;
}

#ifdef Q_OS_UNIX
bool CommandLineUtility::launchAsDaemon()
{
    return daemonMode;
}

QString CommandLineUtility::getDisplayString()
{
    return displayString;
}

#endif

Logger::LogLevel CommandLineUtility::getCurrentLogLevel()
{
    return currentLogLevel;
}

QString CommandLineUtility::getCurrentLogFile() {
  return currentLogFile;
}

QString CommandLineUtility::getErrorText()
{
    return errorText;
}

void CommandLineUtility::setErrorMessage(QString temp)
{
    errorText = temp;
    encounteredError = true;
}

QList<ControllerOptionsInfo>* CommandLineUtility::getControllerOptionsList()
{
    return &controllerOptionsList;
}

bool CommandLineUtility::hasProfileInOptions()
{
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
