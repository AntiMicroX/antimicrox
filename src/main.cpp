#include <QtGlobal>

#ifdef Q_OS_WIN32
  #include <SDL/SDL.h>
  #undef main
#endif

#include <QApplication>
#include <QMainWindow>
#include <QHash>
#include <QHashIterator>
#include <QDir>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QLocalSocket>

#ifdef Q_OS_WIN32
#include <QStyle>
#include <QStyleFactory>
#endif

#include "joystick.h"
#include "joybuttonslot.h"
#include "inputdaemon.h"
#include "common.h"
#include "advancebuttondialog.h"
#include "commandlineutility.h"
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    qRegisterMetaType<JoyButtonSlot*>();
    qRegisterMetaType<AdvanceButtonDialog*>();
    qRegisterMetaType<Joystick*>();

    // If running Win version, check if an explicit style
    // was defined on the command-line. If so, make a note
    // of it.
#ifdef Q_OS_WIN32
    bool styleChangeFound = false;
    for (int i=0; i < argc && !styleChangeFound; i++)
    {
        char *tempchrstr = argv[i];
        QString temp = QString::fromUtf8(tempchrstr);
        if (temp == "-style")
        {
            styleChangeFound = true;
        }
    }
#endif

    QApplication a(argc, argv);
    //QString defaultStyleName = qApp->style()->objectName();

    // If running Win version and no explicit style was
    // defined, use the style Fusion by default. I find the
    // windowsvista style a tad ugly
#ifdef Q_OS_WIN32
    if (!styleChangeFound)
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
    }
#endif

    CommandLineUtility cmdutility;
    QStringList cmdarguments = a.arguments();
    cmdutility.parseArguments(cmdarguments);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("antimicro_" + QLocale::system().name(), QApplication::applicationDirPath().append("/../share/antimicro/translations"));
    a.installTranslator(&myappTranslator);

    if (cmdutility.hasError())
    {
        return 1;
    }
    else if (cmdutility.isHelpRequested())
    {
        cmdutility.printHelp();
        return 0;
    }
    else if (cmdutility.isVersionRequested())
    {
        cmdutility.printVersionString();
        return 0;
    }

    Q_INIT_RESOURCE(resources);
    a.setQuitOnLastWindowClosed(false);

    QDir configDir (PadderCommon::configPath);
    if (!configDir.exists())
    {
        configDir.mkpath(PadderCommon::configPath);
    }

    QHash<int, Joystick*> *joysticks = new QHash<int, Joystick*> ();

    // Cross-platform way of performing IPC. Currently,
    // only establish a connection and then disconnect.
    // In the future, there might be a reason to actually send
    // messages to the QLocalServer.
    QLocalSocket socket;
    socket.connectToServer(PadderCommon::localSocketKey);
    socket.waitForConnected(1000);
    if (socket.state() == QLocalSocket::ConnectedState)
    {
        socket.disconnectFromServer();

        // An instance of this program is already running.
        // Save app config and exit.
        InputDaemon *joypad_worker = new InputDaemon (joysticks, false);
        MainWindow w(joysticks, &cmdutility, false);

        if (!cmdutility.hasError() && cmdutility.hasProfile())
        {
            w.saveAppConfig();
        }

        joypad_worker->quit();
        w.removeJoyTabs();

        QHashIterator<int, Joystick*> iter(*joysticks);
        while (iter.hasNext())
        {
            Joystick *joystick = iter.next().value();
            if (joystick)
            {
                delete joystick;
                joystick = 0;
            }
        }

        joysticks->clear();
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        return 0;
    }

    InputDaemon *joypad_worker = new InputDaemon (joysticks);
    MainWindow w(joysticks, &cmdutility);

    QObject::connect(joypad_worker, SIGNAL(joysticksRefreshed(QHash<int,Joystick*>*)), &w, SLOT(fillButtons(QHash<int,Joystick*>*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(Joystick*)), &w, SLOT(fillButtons(Joystick*)));
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(saveAppConfig()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(removeJoyTabs()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));

    if (!cmdutility.isHiddenRequested() && (!cmdutility.isLaunchInTrayEnabled() || !QSystemTrayIcon::isSystemTrayAvailable()))
    {
        w.show();
    }

    int app_result = a.exec();

    QHashIterator<int, Joystick*> iter(*joysticks);
    while (iter.hasNext())
    {
        Joystick *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();
    delete joysticks;
    joysticks = 0;

    delete joypad_worker;
    joypad_worker = 0;

    return app_result;
}
