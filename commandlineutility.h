#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QObject>
#include <QStringList>
#include <QRegExp>
#include <QTextStream>

class CommandLineUtility : public QObject
{
    Q_OBJECT
public:
    explicit CommandLineUtility(QObject *parent = 0);

    void parseArguments(QStringList& arguments);
    bool isLaunchInTrayEnabled();
    bool isHelpRequested();
    bool isVersionRequested();

    void printHelp();
    void printVersionString();

protected:
    bool launchInTray;
    bool helpRequest;
    bool versionRequest;

    static QRegExp trayRegexp;
    static QRegExp helpShortRegexp;
    static QRegExp helpLongRegexp;
    static QRegExp versionShortRegexp;
    static QRegExp versionLongRegexp;
    
signals:
    
public slots:
    
};

#endif // COMMANDLINEPARSER_H
