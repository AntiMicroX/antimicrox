#include <QDebug>
#include <QStringListIterator>

#include "commandlineutility.h"
#include "common.h"

QRegExp CommandLineUtility::trayRegexp = QRegExp("--tray");
QRegExp CommandLineUtility::helpShortRegexp = QRegExp("-h");
QRegExp CommandLineUtility::helpLongRegexp = QRegExp("--help");
QRegExp CommandLineUtility::versionShortRegexp = QRegExp("-v");
QRegExp CommandLineUtility::versionLongRegexp = QRegExp("--version");

CommandLineUtility::CommandLineUtility(QObject *parent) :
    QObject(parent)
{
    launchInTray = false;
}

void CommandLineUtility::parseArguments(QStringList& arguments)
{
    QStringListIterator iter(arguments);
    while (iter.hasNext())
    {
        QString temp = iter.next();
        if (helpShortRegexp.exactMatch(temp) || helpLongRegexp.exactMatch(temp))
        {
            helpRequest = true;
        }
        else if (versionShortRegexp.exactMatch(temp) || versionLongRegexp.exactMatch(temp))
        {
            versionRequest = true;
        }
        else if (trayRegexp.exactMatch(temp))
        {
            launchInTray = true;
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
    out << "AntiMicro version " << PadderCommon::programVersion << endl;
    out << "Usage: antimicro [option]" << endl;
    out << endl;
    out << "Options:" << endl;
    out << "-h, --help   " << " " << "Print help text" << endl;
    out << "-v, --version" << " " << "Print version information" << endl;
    out << "--tray       " << " " << "Launch program in system tray only" << endl;
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
    out << "AntiMicro version " << PadderCommon::programVersion << endl;
}
