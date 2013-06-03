#include <QApplication>
#include <QMainWindow>
#include <QHash>
#include <QHashIterator>
#include <QThread>
#include <QDir>
#include <QDebug>
#include <QListWidget>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSystemTrayIcon>

#include <X11/Xlib.h>

#include "mainwindow.h"
#include "joystick.h"
#include "joybuttonslot.h"
#include "inputdaemon.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "common.h"
#include "advancebuttondialog.h"
#include "commandlineutility.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<JoyButtonSlot*>();
    qRegisterMetaType<AdvanceButtonDialog*>();

    XInitThreads ();

    QApplication a(argc, argv);
    CommandLineUtility cmdutility;
    QStringList cmdarguments = a.arguments();
    cmdutility.parseArguments(cmdarguments);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("antimicro_" + QLocale::system().name(), QApplication::applicationDirPath().append("/../share/antimicro/translations"));
    a.installTranslator(&myappTranslator);

    if (cmdutility.isHelpRequested())
    {
        cmdutility.printHelp();
        return 0;
    }
    else if (cmdutility.isVersionRequested())
    {
        cmdutility.printVersionString();
        return 0;
    }

    //Q_INIT_RESOURCE(resources);
    //a.setQuitOnLastWindowClosed(false);

    QHash<int, Joystick*> *joysticks = new QHash<int, Joystick*> ();
    MainWindow w(joysticks);

    QDir configDir (PadderCommon::configPath);
    if (!configDir.exists())
    {
        configDir.mkdir(PadderCommon::configPath);
    }

    InputDaemon *joypad_worker = new InputDaemon (joysticks);

    QObject::connect(joypad_worker, SIGNAL(joysticksRefreshed(QHash<int,Joystick*>*)), &w, SLOT(fillButtons(QHash<int,Joystick*>*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(Joystick*)), &w, SLOT(fillButtons(Joystick*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested(Joystick*)), joypad_worker, SLOT(refreshJoystick(Joystick*)));
    //QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(saveAppConfig()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));

    if (!cmdutility.isLaunchInTrayEnabled() || !QSystemTrayIcon::isSystemTrayAvailable())
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

