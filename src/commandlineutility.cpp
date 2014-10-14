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
#ifdef Q_OS_UNIX
QRegExp CommandLineUtility::daemonRegexp = QRegExp("--daemon|-d");
    #ifdef WITH_X11
        QRegExp CommandLineUtility::displayRegexp = QRegExp("--display");
    #endif
#endif

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
}

void CommandLineUtility::parseArguments(QStringList& arguments)
{
    QStringListIterator iter(arguments);
    //QTextStream out(stdout);
    QTextStream errorsteam(stderr);

    while (iter.hasNext())
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
                        errorsteam << tr("Profile location %1 is not an XML file.").arg(temp) << endl;
                        encounteredError = true;
                    }
                    else
                    {
                        profileLocation = fileInfo.absoluteFilePath();
                    }
                }
                else
                {
                    errorsteam << tr("Profile location %1 does not exist.").arg(temp) << endl;
                    encounteredError = true;
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
                    controllerNumber = tempNumber;
                }
                else if (!temp.isEmpty())
                {
                    controllerIDString = temp;
                }
                else
                {
                    errorsteam << tr("Controller identifier is not a valid value.") << endl;
                    encounteredError = true;
                }
            }
        }
        else if (hiddenRegexp.exactMatch(temp))
        {
            hiddenRequest = true;
        }
        else if (unloadRegexp.exactMatch(temp))
        {
            unloadProfile = true;
            profileLocation = "";

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
                    }
                    else if (!temp.isEmpty())
                    {
                        controllerIDString = temp;
                    }
                    else
                    {
                        errorsteam << tr("Controller identifier is not a valid value.") << endl;
                        encounteredError = true;
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
                }
                else if (validNumber)
                {
                    errorsteam << tr("An invalid set number was specified.") << endl;
                    encounteredError = true;
                }

                if (iter.hasNext())
                {
                    temp = iter.next();

                    if (!isPossibleCommand(temp))
                    {
                        if (validNumber)
                        {
                            controllerNumber = tempNumber;
                        }
                        else if (!temp.isEmpty())
                        {
                            controllerIDString = temp;
                        }
                        else
                        {
                            errorsteam << tr("Controller identifier is not a valid value.") << endl;
                            encounteredError = true;
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
                errorsteam << tr("No set number was specified.") << endl;
                encounteredError = true;
            }
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
                    errorsteam << tr("Controller identifier is not a valid value.") << endl;
                    encounteredError = true;
                }
            }
            else
            {
                errorsteam << tr("No controller was specified.") << endl;
                encounteredError = true;
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
                errorsteam << tr("No display string was specified.") << endl;
                encounteredError = true;
            }
        }
    #endif
#endif
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
                    errorsteam << tr("Profile location %1 is not an XML file.").arg(temp) << endl;
                    encounteredError = true;
                }
                else
                {
                    profileLocation = fileInfo.absoluteFilePath();
                }
            }
            else
            {
                errorsteam << tr("Profile location %1 does not exist.").arg(temp) << endl;
                encounteredError = true;
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
    out << tr("AntiMicro version") << " " << PadderCommon::programVersion << endl;
    out << tr("Usage: antimicro [options] [profile]") << endl;
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
#ifdef Q_OS_UNIX
    out << "-d, --daemon                  " << " "
        << tr("Launch program as a daemon.") << endl;
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

#ifdef USE_SDL_2
    out << "-l, --list                    " << " "
        << tr("Print information about joysticks detected by SDL.") << endl;
    out << "--map <value>                 " << " "
        << tr("Open game controller mapping window of selected\n"
              "                               controller. Value can be a controller index or\n"
              "                               GUID.")
        << endl;
#endif

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
    out << tr("AntiMicro version") << " " << PadderCommon::programVersion << endl;
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
    return startSetNumber-1;
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
