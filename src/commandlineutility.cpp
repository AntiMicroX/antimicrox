#include <QDebug>
#include <QStringListIterator>
#include <QFileInfo>
#include <QTextStream>

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


CommandLineUtility::CommandLineUtility(QObject *parent) :
    QObject(parent)
{
    launchInTray = false;
    helpRequest = false;
    versionRequest = false;
    hideTrayIcon = false;
    profileLocation = QString();
    controllerNumber = 0;
    encounteredError = false;
    hiddenRequest = false;
    unloadProfile = false;
    startSetNumber = 0;
}

void CommandLineUtility::parseArguments(QStringList& arguments)
{
    QStringListIterator iter(arguments);
    QTextStream out(stdout);
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
                    if (fileInfo.suffix() != "xml")
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

                if (!temp.startsWith("--") && !temp.startsWith("-"))
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
            else
            {
                errorsteam << tr("No set number was specified.") << endl;
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
    out << tr("Usage: antimicro [options]") << endl;
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
