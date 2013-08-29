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
#include <QTextStream>

#include <sys/file.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "mainwindow.h"
#include "joystick.h"
#include "joybuttonslot.h"
#include "inputdaemon.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "common.h"
#include "advancebuttondialog.h"
#include "commandlineutility.h"

MainWindow *appWindow = 0;

void catchSIGUSR1(int sig) {
    if (appWindow)
    {
        appWindow->loadAppConfig(true);
    }

    signal(sig, catchSIGUSR1);
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<JoyButtonSlot*>();
    qRegisterMetaType<AdvanceButtonDialog*>();
    qRegisterMetaType<Joystick*>();

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

    QFile pidFile(PadderCommon::pidFilePath);
    pidFile.open(QIODevice::ReadWrite);
    int pid_file = pidFile.handle();
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc)
    {
        if (EWOULDBLOCK == errno)
        {
            // An instance of this program is already running.
            // Save app config and exit.
            InputDaemon *joypad_worker = new InputDaemon (joysticks, false);
            MainWindow w(joysticks, &cmdutility, false);
            appWindow = &w;

            if (!cmdutility.hasError() && cmdutility.hasProfile())
            {
                w.saveAppConfig();
            }

            if (pidFile.isOpen())
            {
                int otherpid = 0;
                QTextStream(&pidFile) >> otherpid;
                kill(otherpid, SIGUSR1);
                pidFile.close();
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
    }
    else
    {
        QTextStream(&pidFile) << getpid();
    }

    InputDaemon *joypad_worker = new InputDaemon (joysticks);
    MainWindow w(joysticks, &cmdutility);
    appWindow = &w;

    signal(SIGUSR1, catchSIGUSR1);

    QObject::connect(joypad_worker, SIGNAL(joysticksRefreshed(QHash<int,Joystick*>*)), &w, SLOT(fillButtons(QHash<int,Joystick*>*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(Joystick*)), &w, SLOT(fillButtons(Joystick*)));
    //QObject::connect(&w, SIGNAL(joystickRefreshRequested(Joystick*)), joypad_worker, SLOT(refreshJoystick(Joystick*)));
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(saveAppConfig()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(removeJoyTabs()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));

    if (!cmdutility.isHiddenRequested() && (!cmdutility.isLaunchInTrayEnabled() || !QSystemTrayIcon::isSystemTrayAvailable()))
    {
        w.show();
    }

    int app_result = a.exec();

    if (pidFile.isOpen())
    {
        ftruncate(pidFile.handle(), 0);
        flock(pidFile.handle(), LOCK_UN);
        pidFile.close();
        pidFile.remove();
    }

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
