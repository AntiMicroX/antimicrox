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
                else
                {
                    errorsteam << tr("Controller number is not a valid number.") << endl;
                    encounteredError = true;
                }
            }
        }
        else if (hiddenRegexp.exactMatch(temp))
        {
            hiddenRequest = true;
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
    out << tr("Usage: antimicro [option]") << endl;
    out << endl;
    out << tr("Options") << ":" << endl;
    out << "-h, --help                 " << " " << tr("Print help text.") << endl;
    out << "-v, --version              " << " " << tr("Print version information.") << endl;
    out << "--tray                     " << " " << tr("Launch program in system tray only.") << endl;
    out << "--no-tray                  " << " " << tr("Launch program with the tray menu disabled.") << endl;
    out << "--hidden                   " << " " << tr("Launch program without the main window displayed.") << endl;
    out << "--profile location         " << " " <<
           tr("Launch program with the configuration file\n                            selected as the default for all available\n                            controllers.")
        << endl;
    out << "--profile-controller number" << " " << tr("Apply configuration file to a specific controller.") << endl;
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
