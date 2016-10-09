/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <QThread>

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
#include "applaunchhelper.h"

#include "eventhandlerfactory.h"

#ifndef Q_OS_WIN
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

  #ifdef WITH_X11
    #include "x11extras.h"
  #endif

#endif

#include "antkeymapper.h"
#include "logger.h"

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
    qRegisterMetaType<SetJoystick*>();
    qRegisterMetaType<InputDevice*>();
    qRegisterMetaType<AutoProfileInfo*>();
    qRegisterMetaType<QThread*>();
    qRegisterMetaType<SDL_JoystickID>("SDL_JoystickID");
    qRegisterMetaType<JoyButtonSlot::JoySlotInputAction>("JoyButtonSlot::JoySlotInputAction");

#if defined(Q_OS_UNIX) && defined(WITH_X11)
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    XInitThreads();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif

    QFile logFile;
    QTextStream logFileStream;
    
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

    Logger appLogger(&outstream, &errorstream);

    if (cmdutility.hasError())
    {
        appLogger.LogError(cmdutility.getErrorText(), true, true);
        return 1;
    }
    else if (cmdutility.isHelpRequested())
    {
        appLogger.LogInfo(cmdutility.generateHelpString(), false, true);
        return 0;
    }
    else if (cmdutility.isVersionRequested())
    {
        appLogger.LogInfo(cmdutility.generateVersionString(), true, true);
        return 0;
    }

    // If a log level wasn't specified at the command-line, then use a default.
    if( cmdutility.getCurrentLogLevel() == Logger::LOG_NONE ) {
      appLogger.setLogLevel( Logger::LOG_WARNING );
    } else if (cmdutility.getCurrentLogLevel() != appLogger.getCurrentLogLevel())
    {
        appLogger.setLogLevel(cmdutility.getCurrentLogLevel());
    }

    if( !cmdutility.getCurrentLogFile().isEmpty() ) {
      appLogger.setCurrentLogFile( cmdutility.getCurrentLogFile() );
      appLogger.setCurrentErrorStream(NULL);
    }

    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
    // If in portable mode, make sure the current directory is the same as the
    // config directory. This is to ensure that all relative paths resolve
    // correctly when loading on startup.
    QDir::setCurrent( PadderCommon::configPath() );
#endif

    QDir configDir(PadderCommon::configPath());
    if (!configDir.exists())
    {
      configDir.mkpath(PadderCommon::configPath());
    }

    QMap<SDL_JoystickID, InputDevice*> *joysticks = new QMap<SDL_JoystickID, InputDevice*>();
    QThread *inputEventThread = 0;

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
        AntiMicroSettings settings(PadderCommon::configFilePath(), QSettings::IniFormat);
	
	// Update log info based on config values
	if( cmdutility.getCurrentLogLevel() == Logger::LOG_NONE &&
	    settings.contains("LogLevel")) {
	  appLogger.setLogLevel( (Logger::LogLevel) settings.value("LogLevel").toInt() );
	}
	if( cmdutility.getCurrentLogFile().isEmpty() &&
	    settings.contains("LogFile")) {
	  appLogger.setCurrentLogFile( settings.value("LogFile").toString() );
	  appLogger.setCurrentErrorStream(NULL);
	}

        InputDaemon *joypad_worker = new InputDaemon(joysticks, &settings, false);
        MainWindow w(joysticks, &cmdutility, &settings, false);
        w.fillButtons();
        w.alterConfigFromSettings();

        if (!cmdutility.hasError() &&
           (cmdutility.hasProfile() || cmdutility.hasProfileInOptions()))
        {
            w.saveAppConfig();
        }
        else if (!cmdutility.hasError() && cmdutility.isUnloadRequested())
        {
            w.saveAppConfig();
        }

        w.removeJoyTabs();
        QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));
        QTimer::singleShot(50, &a, SLOT(quit()));

        int result = a.exec();

        settings.sync();
        socket.disconnectFromServer();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete joypad_worker;
        joypad_worker = 0;

        return result;
    }

    LocalAntiMicroServer *localServer = 0;

#ifndef Q_OS_WIN
    if (cmdutility.launchAsDaemon())
    {
        pid_t pid, sid;

        //Fork the Parent Process
        pid = fork();

        if (pid == 0)
        {
            appLogger.LogInfo(QObject::tr("Daemon launched"), true, true);

            localServer = new LocalAntiMicroServer();
            localServer->startLocalServer();
        }
        else if (pid < 0)
        {
            appLogger.LogError(QObject::tr("Failed to launch daemon"), true, true);

            deleteInputDevices(joysticks);
            delete joysticks;
            joysticks = 0;

            exit(EXIT_FAILURE);
        }
        //We got a good pid, Close the Parent Process
        else if (pid > 0)
        {
            appLogger.LogInfo(QObject::tr("Launching daemon"), true, true);

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
            X11Extras::setCustomDisplay(cmdutility.getDisplayString());
            X11Extras::getInstance()->syncDisplay();
            if (X11Extras::getInstance()->display() == NULL)
            {
                appLogger.LogError(QObject::tr("Display string \"%1\" is not valid.")
                                   .arg(cmdutility.getDisplayString()), true, true);
                //errorstream << QObject::tr("Display string \"%1\" is not valid.").arg(cmdutility.getDisplayString()) << endl;

                deleteInputDevices(joysticks);
                delete joysticks;
                joysticks = 0;

                delete localServer;
                localServer = 0;

                X11Extras::getInstance()->closeDisplay();

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
        if (sid < 0)
        {
            appLogger.LogError(QObject::tr("Failed to set a signature id for the daemon"), true, true);
            //errorstream << QObject::tr("Failed to set a signature id for the daemon") << endl;

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
            X11Extras::getInstance()->closeDisplay();

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            }
        #endif
    #endif

            exit(EXIT_FAILURE);
        }

        if ((chdir("/")) < 0)
        {
            appLogger.LogError(QObject::tr("Failed to change working directory to /"), true, true);

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
            X11Extras::getInstance()->closeDisplay();
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
                appLogger.LogError(QObject::tr("Display string \"%1\" is not valid.")
                                   .arg(cmdutility.getDisplayString()), true, true);

                deleteInputDevices(joysticks);
                delete joysticks;
                joysticks = 0;

                delete localServer;
                localServer = 0;

                X11Extras::getInstance()->closeDisplay();

                exit(EXIT_FAILURE);
            }
        }

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif
    #endif
    }

#else
    localServer = new LocalAntiMicroServer();
    localServer->startLocalServer();
#endif

    a.setQuitOnLastWindowClosed(false);

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

    AntiMicroSettings *settings = new AntiMicroSettings(PadderCommon::configFilePath(),
                                                        QSettings::IniFormat);
    settings->importFromCommandLine(cmdutility);

    // Update log info based on config values
    if( cmdutility.getCurrentLogLevel() == Logger::LOG_NONE &&
	settings->contains("LogLevel")) {
      appLogger.setLogLevel( (Logger::LogLevel)settings->value("LogLevel").toInt() );
    }
    if( cmdutility.getCurrentLogFile().isEmpty() &&
	settings->contains("LogFile")) {
      appLogger.setCurrentLogFile( settings->value("LogFile").toString() );
      appLogger.setCurrentErrorStream(NULL);
    }

    QString targetLang = QLocale::system().name();
    if (settings->contains("Language"))
    {
        targetLang = settings->value("Language").toString();
    }

    QTranslator qtTranslator;
#if defined(Q_OS_UNIX)
    qtTranslator.load(QString("qt_").append(targetLang), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#elif defined(Q_OS_WIN)
  #ifdef QT_DEBUG
    qtTranslator.load(QString("qt_").append(targetLang), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  #else
    qtTranslator.load(QString("qt_").append(targetLang),
                      QApplication::applicationDirPath().append("\\share\\qt\\translations"));
  #endif
#endif
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
#if defined(Q_OS_UNIX)
    myappTranslator.load(QString("antimicro_").append(targetLang), QApplication::applicationDirPath().append("/../share/antimicro/translations"));
#elif defined(Q_OS_WIN)
    myappTranslator.load(QString("antimicro_").append(targetLang), QApplication::applicationDirPath().append("\\share\\antimicro\\translations"));
#endif
    a.installTranslator(&myappTranslator);

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

    if (cmdutility.shouldListControllers())
    {
        InputDaemon *joypad_worker = new InputDaemon(joysticks, settings, false);
        AppLaunchHelper mainAppHelper(settings, false);
        mainAppHelper.printControllerList(joysticks);

        joypad_worker->quit();
        joypad_worker->deleteJoysticks();

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
        X11Extras::getInstance()->closeDisplay();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        #endif
    #endif

        return 0;
    }

#ifdef USE_SDL_2
    else if (cmdutility.shouldMapController())
    {
        PadderCommon::mouseHelperObj.initDeskWid();
        InputDaemon *joypad_worker = new InputDaemon(joysticks, settings);
        inputEventThread = new QThread();


        MainWindow *w = new MainWindow(joysticks, &cmdutility, settings);

        QObject::connect(&a, SIGNAL(aboutToQuit()), w, SLOT(removeJoyTabs()));
        QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));
        QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker,
                         SLOT(deleteJoysticks()), Qt::BlockingQueuedConnection);
        QObject::connect(&a, SIGNAL(aboutToQuit()), &PadderCommon::mouseHelperObj,
                         SLOT(deleteDeskWid()), Qt::DirectConnection);
        QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(deleteLater()),
                         Qt::BlockingQueuedConnection);

        //JoyButton::establishMouseTimerConnections();
        w->makeJoystickTabs();
        QTimer::singleShot(0, w, SLOT(controllerMapOpening()));

        joypad_worker->startWorker();

        joypad_worker->moveToThread(inputEventThread);
        PadderCommon::mouseHelperObj.moveToThread(inputEventThread);
        inputEventThread->start(QThread::HighPriority);

        int app_result = a.exec();

        // Log any remaining messages if they exist.
        appLogger.Log();

        inputEventThread->quit();
        inputEventThread->wait();

        delete joysticks;
        joysticks = 0;

        //delete joypad_worker;
        joypad_worker = 0;

        delete localServer;
        localServer = 0;

        delete inputEventThread;
        inputEventThread = 0;

#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
    #endif
        X11Extras::getInstance()->closeDisplay();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
    #endif
#endif

        delete w;
        w = 0;

        return app_result;
    }
#endif

    bool status = true;
    QString eventGeneratorIdentifier;
    AntKeyMapper *keyMapper = 0;
    EventHandlerFactory *factory = EventHandlerFactory::getInstance(cmdutility.getEventGenerator());
    if (!factory)
    {
        status = false;
    }
    else
    {
        eventGeneratorIdentifier = factory->handler()->getIdentifier();
        keyMapper = AntKeyMapper::getInstance(eventGeneratorIdentifier);
        status = factory->handler()->init();
        factory->handler()->printPostMessages();
    }

#if (defined(Q_OS_UNIX) && defined(WITH_UINPUT) && defined(WITH_XTEST)) || \
     defined(Q_OS_WIN)
    // Use fallback event handler.
    if (!status && cmdutility.getEventGenerator() != EventHandlerFactory::fallBackIdentifier())
    {
        QString eventDisplayName = EventHandlerFactory::handlerDisplayName(
                    EventHandlerFactory::fallBackIdentifier());
        appLogger.LogInfo(QObject::tr("Attempting to use fallback option %1 for event generation.")
                                     .arg(eventDisplayName));

        if (keyMapper)
        {
            keyMapper->deleteInstance();
            keyMapper = 0;
        }

        factory->deleteInstance();
        factory = EventHandlerFactory::getInstance(EventHandlerFactory::fallBackIdentifier());
        if (!factory)
        {
            status = false;
        }
        else
        {
            eventGeneratorIdentifier = factory->handler()->getIdentifier();
            keyMapper = AntKeyMapper::getInstance(eventGeneratorIdentifier);
            status = factory->handler()->init();
            factory->handler()->printPostMessages();
        }
    }
#endif

    if (!status)
    {
        appLogger.LogError(QObject::tr("Failed to open event generator. Exiting."));
        appLogger.Log();

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = 0;

        delete localServer;
        localServer = 0;

        if (keyMapper)
        {
            keyMapper->deleteInstance();
            keyMapper = 0;
        }

#if defined(Q_OS_UNIX) && defined(WITH_X11)
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
  #endif
        X11Extras::getInstance()->closeDisplay();
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
  #endif
#endif

        return EXIT_FAILURE;
    }
    else
    {
        appLogger.LogInfo(QObject::tr("Using %1 as the event generator.")
                          .arg(factory->handler()->getName()));
    }

    PadderCommon::mouseHelperObj.initDeskWid();
    InputDaemon *joypad_worker = new InputDaemon(joysticks, settings);
    inputEventThread = new QThread();

    MainWindow *w = new MainWindow(joysticks, &cmdutility, settings);

    w->setAppTranslator(&qtTranslator);
    w->setTranslator(&myappTranslator);

    AppLaunchHelper mainAppHelper(settings, w->getGraphicalStatus());

    QObject::connect(w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(InputDevice*)),
                     w, SLOT(fillButtons(InputDevice*)));
    QObject::connect(joypad_worker,
                     SIGNAL(joysticksRefreshed(QMap<SDL_JoystickID, InputDevice*>*)),
                     w, SLOT(fillButtons(QMap<SDL_JoystickID, InputDevice*>*)));

    QObject::connect(&a, SIGNAL(aboutToQuit()), localServer, SLOT(close()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), w, SLOT(saveAppConfig()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), w, SLOT(removeJoyTabs()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &mainAppHelper, SLOT(revertMouseThread()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(quit()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(deleteJoysticks()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), joypad_worker, SLOT(deleteLater()));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &PadderCommon::mouseHelperObj, SLOT(deleteDeskWid()),
                     Qt::DirectConnection);

#ifdef Q_OS_WIN
    QObject::connect(&a, SIGNAL(aboutToQuit()), &mainAppHelper, SLOT(appQuitPointerPrecision()));
#endif
    QObject::connect(localServer, SIGNAL(clientdisconnect()), w, SLOT(handleInstanceDisconnect()));

#ifdef USE_SDL_2
    QObject::connect(w, SIGNAL(mappingUpdated(QString,InputDevice*)),
                     joypad_worker, SLOT(refreshMapping(QString,InputDevice*)));
    QObject::connect(joypad_worker, SIGNAL(deviceUpdated(int,InputDevice*)),
                     w, SLOT(testMappingUpdateNow(int,InputDevice*)));

    QObject::connect(joypad_worker, SIGNAL(deviceRemoved(SDL_JoystickID)),
                     w, SLOT(removeJoyTab(SDL_JoystickID)));
    QObject::connect(joypad_worker, SIGNAL(deviceAdded(InputDevice*)),
                     w, SLOT(addJoyTab(InputDevice*)));
#endif

#ifdef Q_OS_WIN
    // Raise process priority. Helps reduce timer delays caused by
    // the running of other processes.
    bool raisedPriority = WinExtras::raiseProcessPriority();
    if (!raisedPriority)
    {
        appLogger.LogInfo(QObject::tr("Could not raise process priority."));
    }
#endif

    mainAppHelper.initRunMethods();
    QTimer::singleShot(0, w, SLOT(fillButtons()));
    QTimer::singleShot(0, w, SLOT(alterConfigFromSettings()));
    QTimer::singleShot(0, w, SLOT(changeWindowStatus()));

    mainAppHelper.changeMouseThread(inputEventThread);

    joypad_worker->startWorker();

    joypad_worker->moveToThread(inputEventThread);
    PadderCommon::mouseHelperObj.moveToThread(inputEventThread);
    inputEventThread->start(QThread::HighPriority);

    int app_result = a.exec();

    // Log any remaining messages if they exist.
    appLogger.Log();

    appLogger.LogInfo(QObject::tr("Quitting Program"), true, true);

    joypad_worker = 0;

    delete localServer;
    localServer = 0;

    inputEventThread->quit();
    inputEventThread->wait();

    delete inputEventThread;
    inputEventThread = 0;

    delete joysticks;
    joysticks = 0;

    AntKeyMapper::getInstance()->deleteInstance();

#if defined(Q_OS_UNIX) && defined(WITH_X11)
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    X11Extras::getInstance()->closeDisplay();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif

    EventHandlerFactory::getInstance()->handler()->cleanup();
    EventHandlerFactory::getInstance()->deleteInstance();

    delete w;
    w = 0;

    delete settings;
    settings = 0;

    return app_result;
}
