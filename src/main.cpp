#include <QtGlobal>

#ifdef Q_OS_WIN
  #ifdef USE_SDL_2
  #include <SDL2/SDL.h>
  #else
  #include <SDL/SDL.h>
  #endif
  #undef main
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMap>
#include <QMapIterator>
#include <QDir>
//#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextStream>
#include <QLocalSocket>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QStyle>
#include <QStyleFactory>
#include "winextras.h"
#endif

#include "inputdevice.h"
#include "joybuttonslot.h"
#include "inputdaemon.h"
#include "common.h"
#include "commandlineutility.h"
#include "mainwindow.h"
#include "autoprofileinfo.h"
#include "localantimicroserver.h"
#include "antimicrosettings.h"

#ifndef Q_OS_WIN
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "eventhandlerfactory.h"

    #ifdef WITH_X11
#include "x11extras.h"
    #endif

#endif

#include "antkeymapper.h"

#ifndef Q_OS_WIN
static void termSignalTermHandler(int signal)
{
    Q_UNUSED(signal);

    qApp->exit(0);
}

static void termSignalIntHandler(int signal)
{
    Q_UNUSED(signal);

    qApp->exit(0);
}

#endif

void deleteInputDevices(QMap<SDL_JoystickID, InputDevice*> *joysticks)
{
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<JoyButtonSlot*>();
    qRegisterMetaType<InputDevice*>();
    qRegisterMetaType<AutoProfileInfo*>();

    QTextStream outstream(stdout);
    QTextStream errorstream(stderr);

    // If running Win version, check if an explicit style
    // was defined on the command-line. If so, make a note
    // of it.
#ifdef Q_OS_WIN
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

    CommandLineUtility cmdutility;
    QStringList cmdarguments = PadderCommon::arguments(argc, argv);
    cmdarguments.removeFirst();
    cmdutility.parseArguments(cmdarguments);

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

    QDir configDir(PadderCommon::configPath);
    if (!configDir.exists())
    {
        configDir.mkpath(PadderCommon::configPath);
    }

    QMap<SDL_JoystickID, InputDevice*> *joysticks = new QMap<SDL_JoystickID, InputDevice*>();

    // Cross-platform way of performing IPC. Currently,
    // only establish a connection and then disconnect.
    // In the future, there might be a reason to actually send
    // messages to the QLocalServer.
    QLocalSocket socket;
    socket.connectToServer(PadderCommon::localSocketKey);
    socket.waitForConnected(1000);
    if (socket.state() == QLocalSocket::ConnectedState)
    {
        // An instance of this program is already running.
        // Save app config and exit.
        QApplication a(argc, argv);
        AntiMicroSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
        InputDaemon *joypad_worker = new InputDaemon(joysticks, &settings, false);
        MainWindow w(joysticks, &cmdutility, &settings, false);

        if (!cmdutility.hasError() && cmdutility.hasProfile())
        {
            w.saveAppConfig();
        }

        joypad_worker->quit();
        w.removeJoyTabs();

        settings.sync();
        socket.disconnectFromServer();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        return 0;
    }

    LocalAntiMicroServer *localServer = 0;
    QApplication *a = 0;

#ifndef Q_OS_WIN
    if (cmdutility.launchAsDaemon())
    {
        pid_t pid, sid;

        //Fork the Parent Process
        pid = fork();

        if (pid == 0)
        {
            outstream << QObject::tr("Daemon launched") << endl;

            a = new QApplication(argc, argv);
            localServer = new LocalAntiMicroServer();
            localServer->startLocalServer();
        }
        else if (pid < 0) {

            errorstream << QObject::tr("Failed to launch daemon") << endl;

            deleteInputDevices(joysticks);
            delete joysticks;
            joysticks = 0;

            exit(EXIT_FAILURE);
        }
        //We got a good pid, Close the Parent Process
        else if (pid > 0) {
            outstream << QObject::tr("Launching daemon") << endl;

            deleteInputDevices(joysticks);
            delete joysticks;
            joysticks = 0;

            exit(EXIT_SUCCESS);
        }


    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif

        if (cmdutility.getDisplayString().isEmpty())
        {
            X11Extras::getInstance()->syncDisplay();
        }
        else
        {
            X11Extras::getInstance()->syncDisplay(cmdutility.getDisplayString());
            if (X11Extras::getInstance()->display() == NULL)
            {
                errorstream << QObject::tr("Display string \"%1\" is not valid.").arg(cmdutility.getDisplayString()) << endl;

                deleteInputDevices(joysticks);
                delete joysticks;
                joysticks = 0;

                delete localServer;
                localServer = 0;

                X11Extras::deleteInstance();

                exit(EXIT_FAILURE);
            }
        }

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif

    #endif

        //Change File Mask
        umask(0);

        //Create a new Signature Id for our child
        sid = setsid();
        if (sid < 0) {
            errorstream << QObject::tr("Failed to set a signature id for the daemon") << endl;

            deleteInputDevices(joysticks);
            delete joysticks;
            joysticks = 0;

            delete localServer;
            localServer = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            if (QApplication::platformName() == QStringLiteral("xcb"))
            {
        #endif
            X11Extras::deleteInstance();

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            }
        #endif
    #endif

            exit(EXIT_FAILURE);
        }

        if ((chdir("/")) < 0) {
            errorstream << QObject::tr("Failed to change working directory to /")
                        << endl;

            deleteInputDevices(joysticks);
            delete joysticks;
            joysticks = 0;

            delete localServer;
            localServer = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

            if (QApplication::platformName() == QStringLiteral("xcb"))
            {
        #endif
            X11Extras::deleteInstance();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            }
        #endif
    #endif

            exit(EXIT_FAILURE);
        }

        //Close Standard File Descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    else
    {
        a = new QApplication(argc, argv);
        localServer = new LocalAntiMicroServer();
        localServer->startLocalServer();

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif
        if (!cmdutility.getDisplayString().isEmpty())
        {
            X11Extras::getInstance()->syncDisplay(cmdutility.getDisplayString());
            if (X11Extras::getInstance()->display() == NULL)
            {
                errorstream << QObject::tr("Display string \"%1\" is not valid.").arg(cmdutility.getDisplayString()) << endl;

                deleteInputDevices(joysticks);
                delete joysticks;
                joysticks = 0;

                delete localServer;
                localServer = 0;

                X11Extras::deleteInstance();

                exit(EXIT_FAILURE);
            }
        }

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif
    #endif
    }

#else
    a = new QApplication (argc, argv);
    localServer = new LocalAntiMicroServer();
    localServer->startLocalServer();
#endif

    a->setQuitOnLastWindowClosed(false);

    //QString defaultStyleName = qApp->style()->objectName();

    // If running Win version and no explicit style was
    // defined, use the style Fusion by default. I find the
    // windowsvista style a tad ugly
#ifdef Q_OS_WIN
    if (!styleChangeFound)
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
    }

    QIcon::setThemeName("/");
#endif

    QTranslator qtTranslator;
    qtTranslator.load(QString("qt_").append(QLocale::system().name()), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a->installTranslator(&qtTranslator);

    QTranslator myappTranslator;
#if defined(Q_OS_UNIX)
    myappTranslator.load(QString("antimicro_").append(QLocale::system().name()), QApplication::applicationDirPath().append("/../share/antimicro/translations"));
#elif defined(Q_OS_WIN)
    myappTranslator.load(QString("antimicro_").append(QLocale::system().name()), QApplication::applicationDirPath().append("\\share\\antimicro\\translations"));
#endif
    a->installTranslator(&myappTranslator);

    //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    AntiMicroSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    settings.importFromCommandLine(cmdutility);
    InputDaemon *joypad_worker = new InputDaemon(joysticks, &settings);

#ifndef Q_OS_WIN
    // Have program handle SIGTERM
    struct sigaction termaction;
    termaction.sa_handler = &termSignalTermHandler;
    sigemptyset(&termaction.sa_mask);
    termaction.sa_flags = 0;

    sigaction(SIGTERM, &termaction, 0);

    // Have program handle SIGINT
    struct sigaction termint;
    termint.sa_handler = &termSignalIntHandler;
    sigemptyset(&termint.sa_mask);
    termint.sa_flags = 0;

    sigaction(SIGINT, &termint, 0);

#endif

#ifdef USE_SDL_2
    if (cmdutility.shouldListControllers())
    {
        outstream << QObject::tr("# of joysticks found: %1").arg(joysticks->size()) << endl;
        outstream << endl;
        outstream << QObject::tr("List Joysticks:") << endl;
        outstream << QObject::tr("---------------") << endl;
        QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
        unsigned int indexNumber = 1;
        while (iter.hasNext())
        {
            InputDevice *tempdevice = iter.next().value();
            outstream << QObject::tr("Joystick %1:").arg(indexNumber) << endl;
            outstream << "  " << QObject::tr("Index:           %1").arg(tempdevice->getRealJoyNumber()) << endl;
            outstream << "  " << QObject::tr("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
            outstream << "  " << QObject::tr("Name:            %1").arg(tempdevice->getSDLName()) << endl;
            QString gameControllerStatus = tempdevice->isGameController() ?
                                           QObject::tr("Yes") : QObject::tr("No");
            outstream << "  " << QObject::tr("Game Controller: %1").arg(gameControllerStatus) << endl;
            outstream << "  " << QObject::tr("# of Axes:       %1").arg(tempdevice->getNumberRawAxes()) << endl;
            outstream << "  " << QObject::tr("# of Buttons:    %1").arg(tempdevice->getNumberRawButtons()) << endl;
            outstream << "  " << QObject::tr("# of Hats:       %1").arg(tempdevice->getNumberHats()) << endl;

            if (iter.hasNext())
            {
                outstream << endl;
                indexNumber++;
            }
        }

        joypad_worker->quit();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        delete localServer;
        localServer = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif
        X11Extras::deleteInstance();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif
    #endif

        delete a;
        a = 0;

        return 0;
    }
    else if (cmdutility.shouldMapController())
    {
        MainWindow *w = new MainWindow(joysticks, &cmdutility, &settings);

        QObject::connect(a, SIGNAL(aboutToQuit()), w, SLOT(removeJoyTabs()));
        QObject::connect(a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));

        int app_result = a->exec();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        delete localServer;
        localServer = 0;

#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
    #endif
        X11Extras::deleteInstance();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
    #endif
#endif

        delete w;
        w = 0;

        delete a;
        a = 0;

        return app_result;
    }
#endif

#ifdef Q_OS_UNIX
    bool status = true;
    EventHandlerFactory *factory = EventHandlerFactory::getInstance(cmdutility.getEventGenerator());
    if (!factory)
    {
        status = false;
    }
    else
    {
        status = factory->handler()->init();
    }

#if defined(WITH_UINPUT) && defined(WITH_XTEST)
    // Use xtest as a fallback.
    if (!status && cmdutility.getEventGenerator() != EventHandlerFactory::fallBackIdentifier())
    {
        QString eventDisplayName = EventHandlerFactory::handlerDisplayName(
                    EventHandlerFactory::fallBackIdentifier());
        outstream << QObject::tr("Attempting to use fallback option %1 for event generation.")
                     .arg(eventDisplayName) << endl;

        factory->deleteInstance();
        factory = EventHandlerFactory::getInstance(EventHandlerFactory::fallBackIdentifier());
        if (!factory)
        {
            status = false;
        }
        else
        {
            status = factory->handler()->init();
        }
    }
#endif

    if (!status)
    {
        errorstream << QObject::tr("Failed to open event generator. Exiting.") << endl;

        joypad_worker->quit();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        delete localServer;
        localServer = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        #endif
        X11Extras::deleteInstance();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif
    #endif

        delete a;
        a = 0;

        return EXIT_FAILURE;
    }
    else
    {
        outstream << QObject::tr("Using %1 as the event generator.").arg(factory->handler()->getName())
                  << endl;
        factory->handler()->printPostMessages();
    }
#endif

    AntKeyMapper::getInstance(cmdutility.getEventGenerator());

    MainWindow *w = new MainWindow(joysticks, &cmdutility, &settings);

    QObject::connect(joypad_worker, SIGNAL(joysticksRefreshed(QMap<SDL_JoystickID, InputDevice*>*)), w, SLOT(fillButtons(QMap<SDL_JoystickID, InputDevice*>*)));
    QObject::connect(w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(InputDevice*)), w, SLOT(fillButtons(InputDevice*)));
    QObject::connect(a, SIGNAL(aboutToQuit()), localServer, SLOT(close()));
    QObject::connect(a, SIGNAL(aboutToQuit()), w, SLOT(saveAppConfig()));
    QObject::connect(a, SIGNAL(aboutToQuit()), w, SLOT(removeJoyTabs()));
    QObject::connect(a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));
    QObject::connect(localServer, SIGNAL(clientdisconnect()), w, SLOT(handleInstanceDisconnect()));

#ifdef USE_SDL_2
    QObject::connect(w, SIGNAL(mappingUpdated(QString,InputDevice*)), joypad_worker, SLOT(refreshMapping(QString,InputDevice*)));
    QObject::connect(joypad_worker, SIGNAL(deviceUpdated(int,InputDevice*)), w, SLOT(testMappingUpdateNow(int,InputDevice*)));
    QObject::connect(joypad_worker, SIGNAL(deviceRemoved(SDL_JoystickID)), w, SLOT(removeJoyTab(SDL_JoystickID)));
    QObject::connect(joypad_worker, SIGNAL(deviceAdded(InputDevice*)), w, SLOT(addJoyTab(InputDevice*)));
#endif


#ifdef Q_OS_WIN
    // Raise process priority. Helps reduce timer delays caused by
    // the running of other processes.
    bool raisedPriority = WinExtras::raiseProcessPriority();
    if (!raisedPriority)
    {
        outstream << QObject::tr("Could not raise process priority.") << endl;
    }
#else
    // Raise main thread prority. Helps reduce timer delays caused by
    // the running of other processes.
    QThread::currentThread()->setPriority(QThread::HighPriority);
#endif
    int app_result = a->exec();

    deleteInputDevices(joysticks);
    delete joysticks;
    joysticks = 0;

    delete joypad_worker;
    joypad_worker = 0;

    delete localServer;
    localServer = 0;

    AntKeyMapper::getInstance()->deleteInstance();

#ifdef Q_OS_UNIX
    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    X11Extras::deleteInstance();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #endif

    EventHandlerFactory::getInstance()->handler()->cleanup();
    EventHandlerFactory::getInstance()->deleteInstance();
#endif

    delete w;
    w = 0;

    delete a;
    a = 0;

    return app_result;
}
