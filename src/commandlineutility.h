#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QObject>
#include <QStringList>
#include <QRegExp>

class CommandLineUtility : public QObject
{
    Q_OBJECT
public:
    explicit CommandLineUtility(QObject *parent = 0);

    void parseArguments(QStringList& arguments);
    bool isLaunchInTrayEnabled();
    bool isHelpRequested();
    bool isVersionRequested();
    bool isTrayHidden();
    bool hasProfile();
    bool hasControllerNumber();
    bool hasControllerID();

    QString getProfileLocation();
    unsigned int getControllerNumber();
    QString getControllerID();
    bool isHiddenRequested();
    bool isUnloadRequested();

    unsigned int getStartSetNumber();
    unsigned int getJoyStartSetNumber();

    void printHelp();
    void printVersionString();

    bool hasError();

protected:
    bool isPossibleCommand(QString temp);

    bool launchInTray;
    bool helpRequest;
    bool versionRequest;
    bool hideTrayIcon;
    QString profileLocation;
    unsigned int controllerNumber;
    QString controllerIDString;
    bool encounteredError;
    bool hiddenRequest;
    bool unloadProfile;
    unsigned int startSetNumber;

    static QRegExp trayRegexp;
    static QRegExp helpRegexp;
    static QRegExp versionRegexp;
    static QRegExp noTrayRegexp;
    static QRegExp loadProfileRegexp;
    static QRegExp loadProfileForControllerRegexp;
    static QRegExp hiddenRegexp;
    static QRegExp unloadRegexp;
    static QRegExp startSetRegexp;
    
signals:
    
public slots:
    
};

#endif // COMMANDLINEPARSER_H
