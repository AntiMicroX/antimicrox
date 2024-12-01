/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2021 Paweł Kotiuk
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

#include "antimicrosettings.h"
#include "antkeymapper.h"
#include "applaunchhelper.h"
#include "autoprofileinfo.h"
#include "commandlineutility.h"
#include "common.h"
#include "inputdaemon.h"
#include "inputdevice.h"
#include "joybuttonslot.h"
#include "joysensordirection.h"
#include "joysensortype.h"
#include "localantimicroserver.h"
#include "mainwindow.h"
#include "setjoystick.h"
#include "simplekeygrabberbutton.h"

#include "eventhandlerfactory.h"
#include "logger.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QException>
#include <QLibraryInfo>
#include <QLocalSocket>
#include <QMainWindow>
#include <QMap>
#include <QMapIterator>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>
#include <QTranslator>
#include <QtGlobal>

#include <iostream>
#include <stdexcept>

#ifdef Q_OS_UNIX
    #include <signal.h>
    #include <unistd.h>

    #include <execinfo.h>

    #ifdef WITH_X11
        #include "x11extras.h"
    #endif

static void termSignalTermHandler(int signal)
{
    Q_UNUSED(signal)
    qDebug() << "Received SIGTERM. Closing...";
    qApp->exit(0);
}

static void termSignalIntHandler(int signal)
{
    Q_UNUSED(signal)
    qDebug() << "Received SIGINT. Closing...";
    qApp->exit(0);
}

static void termSignalSegfaultHandler(int signal)
{
    if (signal == SIGSEGV)
        ERROR() << "Received SIGSEGV (segmentation fault)";
    else if (signal == SIGABRT)
        ERROR() << "Received SIGABRT (abort)";
    else
        ERROR() << "Received signal with number" << signal;
    const int MAX_NUM = 32;
    void *array[MAX_NUM];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, MAX_NUM);

    char **strings = backtrace_symbols(array, size);
    WARN() << "Stack trace:";
    for (size_t i = 0; i < size; i++)
    {
        WARN() << strings[i] << "\t";
    }
    free(strings);

    // Restore default handler
    struct sigaction segint;
    segint.sa_handler = SIG_DFL;
    sigemptyset(&segint.sa_mask);
    segint.sa_flags = 0;
    sigaction(signal, &segint, nullptr);

    delete Logger::getInstance();
}

void installSignalHandlers()
{
    // Have program handle SIGTERM
    struct sigaction termaction;
    termaction.sa_handler = &termSignalTermHandler;
    sigemptyset(&termaction.sa_mask);
    termaction.sa_flags = 0;

    sigaction(SIGTERM, &termaction, nullptr);

    // Have program handle SIGINT
    struct sigaction termint;
    termint.sa_handler = &termSignalIntHandler;
    sigemptyset(&termint.sa_mask);
    termint.sa_flags = 0;

    sigaction(SIGINT, &termint, nullptr);

    // Have program handle SIGSEGV
    struct sigaction segint;
    segint.sa_handler = &termSignalSegfaultHandler;
    sigemptyset(&segint.sa_mask);
    segint.sa_flags = 0;

    sigaction(SIGSEGV, &segint, nullptr);

    // Have program handle SIGABRT
    struct sigaction segabrt;
    segabrt.sa_handler = &termSignalSegfaultHandler;
    sigemptyset(&segabrt.sa_mask);
    segabrt.sa_flags = 0;

    sigaction(SIGABRT, &segabrt, nullptr);
}
#endif

// was non static
static void deleteInputDevices(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    QMapIterator<SDL_JoystickID, InputDevice *> iter(*joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();

        if (joystick != nullptr)
        {
            delete joystick;
            joystick = nullptr;
        }
    }

    joysticks->clear();
}

/**
 * @brief Function used for copying settings used by antimicro and
 * previous revisions of antimicrox to provide backward compatibility
 */
void importLegacySettingsIfExist()
{
    const QFileInfo config(PadderCommon::configFilePath());
    const bool configExists = config.exists() && config.isFile();
    if (configExists)
    {
        DEBUG() << "Found settings file: " << PadderCommon::configFilePath();
        return;
    }
    // 'antimicroX'
    const QFileInfo legacyConfig(PadderCommon::configLegacyFilePath());
    const bool legacyConfigExists = legacyConfig.exists() && legacyConfig.isFile();
    // 'antimicro'
    const QFileInfo legacyAntimicroConfig(PadderCommon::configAntimicroLegacyFilePath());
    const bool legacyAntimicroConfigExists = legacyAntimicroConfig.exists() && legacyAntimicroConfig.isFile();

    const bool requireMigration = !configExists && (legacyConfigExists || legacyAntimicroConfigExists);
    if (requireMigration)
    {
        const QFileInfo fileToCopy = legacyConfigExists ? legacyConfig : legacyAntimicroConfig;
#if defined(Q_OS_WIN)
        const QString location = PadderCommon::configPath();
#else
        const QString location = "~/.config/antimicrox";
#endif
        QDir(PadderCommon::configPath()).mkpath(PadderCommon::configPath());
        const bool copySuccess = QFile::copy(fileToCopy.canonicalFilePath(), PadderCommon::configFilePath());
        DEBUG() << "Legacy settings found";
        const QString successMessage =
            QObject::tr("Your original settings (previously stored in %1) have been copied to\n%2\n If you want you can "
                        "delete the original directory or leave it as it is.")
                .arg(fileToCopy.canonicalFilePath(), location);
        const QString errorMessage =
            QObject::tr("Some problem with settings migration occurred.\nOriginal configs are stored in \n%1\n but their "
                        "new location is: \n%2\nYou can migrate manually by renaming old directory and renaming file to "
                        "antimicrox_settings.ini.")
                .arg(fileToCopy.canonicalFilePath(), location);

        QMessageBox msgBox;
        if (copySuccess)
        {
            DEBUG() << "Legacy settings copied";
            msgBox.setText(successMessage);
        } else
        {
            WARN() << "Problem with importing settings from: " << fileToCopy.canonicalFilePath()
                   << " to: " << PadderCommon::configFilePath();
            msgBox.setText(errorMessage);
        }
        msgBox.exec();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(Logger::loggerMessageHandler);

    QApplication antimicrox(argc, argv);
    QCoreApplication::setApplicationName("antimicrox");
    QCoreApplication::setApplicationVersion(PadderCommon::programVersion);

    QTextStream outstream(stdout);
    Logger *appLogger = Logger::createInstance(&outstream, Logger::LogLevel::LOG_DEBUG);

    qRegisterMetaType<JoyButtonSlot *>();
    qRegisterMetaType<SetJoystick *>();
    qRegisterMetaType<InputDevice *>();
    qRegisterMetaType<AutoProfileInfo *>();
    qRegisterMetaType<QThread *>();
    qRegisterMetaType<SDL_JoystickID>("SDL_JoystickID");
    qRegisterMetaType<JoyButtonSlot::JoySlotInputAction>("JoyButtonSlot::JoySlotInputAction");
    qRegisterMetaType<JoySensorType>();
    qRegisterMetaType<JoySensorDirection>();

#if defined(WITH_X11)

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        XInitThreads();
    }

#endif
    importLegacySettingsIfExist();
    AntiMicroSettings settings(PadderCommon::configFilePath(), QSettings::IniFormat);
    CommandLineUtility cmdutility;

    try
    {
        cmdutility.parseArguments(antimicrox);
    } catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << "Closing\n";
        return -1;
    }
    settings.importFromCommandLine(cmdutility);
    settings.applySettingsToLogger(cmdutility, appLogger);

    Q_INIT_RESOURCE(resources);

    QDir configDir(PadderCommon::configPath());

    if (!configDir.exists())
    {
        configDir.mkpath(PadderCommon::configPath());
    }

    QMap<SDL_JoystickID, InputDevice *> *joysticks = new QMap<SDL_JoystickID, InputDevice *>();
    QThread *inputEventThread = nullptr;

    // Cross-platform way of performing IPC. Currently,
    // only establish a connection and then disconnect.
    // In the future, there might be a reason to actually send
    // messages to the QLocalServer.
    QLocalSocket socket;
    PadderCommon::log_system_config();

    if ((socket.serverName() == QString()))
    {
        socket.connectToServer(PadderCommon::localSocketKey);

        if (!socket.waitForConnected(3000))
        {
            qDebug() << "Socket's state: " << socket.state();
            qDebug() << "Server name: " << socket.serverName();
            qDebug() << "Socket descriptor: " << socket.socketDescriptor();
            qDebug() << "The connection hasn't been established: \nerror text -> " << socket.error() << "\nerror text 2 ->"
                     << socket.errorString();
        } else
        {
            qDebug() << "Socket connected";
        }
    } else
    {
        socket.abort();
    }

    if (!socket.isValid())
    {
        qDebug() << "Socket is not valid";
        qDebug() << "Socket's state: " << socket.state();
        qDebug() << "Server name: " << socket.serverName();
        qDebug() << "Socket descriptor: " << socket.socketDescriptor();
    }

    if (socket.state() == QLocalSocket::ConnectedState)
    {
        // An instance of this program is already running.
        // Save app config and exit.
        PRINT_STDOUT() << "AntiMicroX is already running.\n";
        QPointer<InputDaemon> joypad_worker = new InputDaemon(joysticks, &settings, false);
        MainWindow mainWindow(joysticks, &cmdutility, &settings, false);
        mainWindow.fillButtons();
        mainWindow.alterConfigFromSettings();

        if (cmdutility.hasProfile() || cmdutility.hasProfileInOptions())
        {
            PRINT_STDOUT() << "Update selected profile\n";
            mainWindow.saveAppConfig();
        } else if (cmdutility.isUnloadRequested())
        {
            mainWindow.saveAppConfig();
        } else if (cmdutility.isShowRequested())
        {
            INFO() << "Showing window if hidden.\n";
            socket.write(PadderCommon::unhideCommand.toStdString().c_str());
            socket.waitForBytesWritten(100);
        }
        qDebug() << "Closing this app instance";

        mainWindow.removeJoyTabs();
        QObject::connect(&antimicrox, &QApplication::aboutToQuit, joypad_worker.data(), &InputDaemon::quit);

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
        QTimer::singleShot(50, &antimicrox, &QApplication::quit);
#else
        QTimer::singleShot(50, &antimicrox, SLOT(quit()));
#endif

        int result = antimicrox.exec();

        settings.sync();
        socket.disconnectFromServer();
        if (socket.state() == QLocalSocket::LocalSocketState::ConnectedState ||
            socket.state() == QLocalSocket::LocalSocketState::ClosingState)
        {
            if (socket.waitForDisconnected(2000))
                qDebug() << "Socket " << socket.socketDescriptor() << " disconnected!";
        } else
            qDebug() << "Socket " << socket.socketDescriptor() << " disconnected!";
        deleteInputDevices(joysticks);

        delete joysticks;
        joysticks = nullptr;

        if (!joypad_worker.isNull())
        {
            delete joypad_worker;
            joypad_worker.clear();
        }
        delete appLogger;
        return result;
    }

    LocalAntiMicroServer *localServer = new LocalAntiMicroServer();
    localServer->startLocalServer();

#if defined(Q_OS_WIN)
    qApp->setStyle("fusion");
#endif

    antimicrox.setQuitOnLastWindowClosed(false);

    QStringList appDirsLocations = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
    appDirsLocations.append(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    QStringList themePathsTries = QStringList();

    for (QList<QString>::const_iterator i = appDirsLocations.constBegin(); i != appDirsLocations.constEnd(); ++i)
    {
        themePathsTries.append(QString("%1%2").arg(*i).arg("/icons"));
        qDebug() << QString("%1%2").arg(*i).arg("/icons");
    }

    QIcon::setThemeSearchPaths(themePathsTries);
    qDebug() << "Theme name: " << QIcon::themeName();

    // Update log info based on config values
    if (cmdutility.getCurrentLogLevel() == Logger::LOG_NONE && settings.contains("LogLevel"))
    {
        appLogger->setLogLevel(static_cast<Logger::LogLevel>(settings.value("LogLevel").toInt()));
    }

    if (cmdutility.getCurrentLogFile().isEmpty() && settings.contains("LogFile"))
    {
        appLogger->setCurrentLogFile(settings.value("LogFile").toString());
    }

    QString targetLang = QLocale::system().name();

    if (settings.contains("Language"))
    {
        targetLang = settings.value("Language").toString();
    }

    QTranslator qtTranslator;

#if defined(Q_OS_UNIX)
    // Ensure that the Wayland appId matches the .desktop file name
    QGuiApplication::setDesktopFileName("io.github.antimicrox.antimicrox");

    installSignalHandlers();

    QString transPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    if (QDir(transPath).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0)
    {
        qtTranslator.load(QString("qt_").append(targetLang), "/app/share/antimicrox/translations");
    } else
    {
        qtTranslator.load(QString("qt_").append(targetLang), transPath);
    }

#endif
    antimicrox.installTranslator(&qtTranslator);

    QTranslator myappTranslator;

    if (QDir("/app/share/antimicrox").entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() > 0)
    {
        myappTranslator.load(QString("antimicrox_").append(targetLang), "app/share/antimicrox/translations");
    } else
    {
        myappTranslator.load(QString("antimicrox_").append(targetLang),
                             QApplication::applicationDirPath().append("/../share/antimicrox/translations"));
    }

    antimicrox.installTranslator(&myappTranslator);

    if (cmdutility.shouldListControllers())
    {
        QPointer<InputDaemon> joypad_worker = new InputDaemon(joysticks, &settings, false);
        AppLaunchHelper mainAppHelper(&settings, false);
        mainAppHelper.printControllerList(joysticks);

        joypad_worker->quit();

        delete joysticks;
        joysticks = nullptr;

        delete localServer;
        localServer = nullptr;

#ifdef WITH_X11

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            X11Extras::getInstance()->closeDisplay();
        }

#endif
        delete appLogger;
        return 0;
    }

    bool status = true;
    QString eventGeneratorIdentifier = QString();
    AntKeyMapper *keyMapper = nullptr;
    EventHandlerFactory *factory = EventHandlerFactory::getInstance(cmdutility.getEventGenerator());

    if (!factory)
    {
        status = false;
    } else
    {
        eventGeneratorIdentifier = factory->handler()->getIdentifier();
        keyMapper = AntKeyMapper::getInstance(eventGeneratorIdentifier);
        status = factory->handler()->init();
        factory->handler()->printPostMessages();
    }

#if defined(WITH_UINPUT) && defined(WITH_XTEST)

    // Use fallback event handler.
    if (!status && cmdutility.getEventGenerator() != EventHandlerFactory::fallBackIdentifier())
    {
        QString eventDisplayName = EventHandlerFactory::handlerDisplayName(EventHandlerFactory::fallBackIdentifier());
        qInfo() << QObject::tr("Attempting to use fallback option %1 for event generation.").arg(eventDisplayName);

        if (keyMapper != nullptr)
        {
            keyMapper->deleteInstance();
            keyMapper = nullptr;
        }

        factory->deleteInstance();
        factory = EventHandlerFactory::getInstance(EventHandlerFactory::fallBackIdentifier());

        if (!factory)
        {
            status = false;
        } else
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
        PRINT_STDERR() << QObject::tr("Failed to open event generator. Exiting.") << "\n";

        deleteInputDevices(joysticks);
        delete joysticks;
        joysticks = nullptr;

        delete localServer;
        localServer = nullptr;

        if (keyMapper != nullptr)
        {
            keyMapper->deleteInstance();
            keyMapper = nullptr;
        }

#if defined(WITH_X11)

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            X11Extras::getInstance()->closeDisplay();
        }

#endif
        delete appLogger;
        return EXIT_FAILURE;
    }
    qInfo() << QObject::tr("Using %1 as the event generator.").arg(factory->handler()->getName());
#ifdef Q_OS_WIN
    PadderCommon::log_system_config(); // workaround for missing windows logs
#endif

    QPointer<InputDaemon> joypad_worker = new InputDaemon(joysticks, &settings);
    inputEventThread = new QThread();
    inputEventThread->setObjectName("inputEventThread");

    MainWindow *mainWindow = new MainWindow(joysticks, &cmdutility, &settings);

    mainWindow->setAppTranslator(&qtTranslator);
    mainWindow->setTranslator(&myappTranslator);

    AppLaunchHelper mainAppHelper(&settings, mainWindow->getGraphicalStatus());

    QObject::connect(mainWindow, &MainWindow::joystickRefreshRequested, joypad_worker.data(), &InputDaemon::refresh);
    QObject::connect(joypad_worker.data(), &InputDaemon::joystickRefreshed, mainWindow, &MainWindow::fillButtonsID);
    QObject::connect(joypad_worker.data(), &InputDaemon::joysticksRefreshed, mainWindow, &MainWindow::fillButtonsMap);

    QObject::connect(&antimicrox, &QApplication::aboutToQuit, localServer, &LocalAntiMicroServer::close);
    QObject::connect(&antimicrox, &QApplication::aboutToQuit, mainWindow, &MainWindow::saveAppConfig);
    QObject::connect(&antimicrox, &QApplication::aboutToQuit, mainWindow, &MainWindow::removeJoyTabs);
    QObject::connect(&antimicrox, &QApplication::aboutToQuit, &mainAppHelper, &AppLaunchHelper::revertMouseThread);
    QObject::connect(&antimicrox, &QApplication::aboutToQuit, joypad_worker.data(), &InputDaemon::quit);
    QObject::connect(&antimicrox, &QApplication::aboutToQuit, joypad_worker.data(), &InputDaemon::deleteLater);

    QObject::connect(localServer, &LocalAntiMicroServer::showHiddenWindow, mainWindow, &MainWindow::show);
    QObject::connect(localServer, &LocalAntiMicroServer::clientdisconnect, mainWindow,
                     &MainWindow::handleInstanceDisconnect);
    QObject::connect(mainWindow, &MainWindow::mappingUpdated, joypad_worker.data(), &InputDaemon::refreshMapping);
    QObject::connect(joypad_worker.data(), &InputDaemon::deviceUpdated, mainWindow, &MainWindow::testMappingUpdateNow);

    QObject::connect(joypad_worker.data(), &InputDaemon::deviceRemoved, mainWindow, &MainWindow::removeJoyTab);
    QObject::connect(joypad_worker.data(), &InputDaemon::deviceAdded, mainWindow, &MainWindow::addJoyTab);

    mainAppHelper.initRunMethods();

    QTimer::singleShot(0, mainWindow, SLOT(fillButtons()));
    QTimer::singleShot(0, mainWindow, SLOT(alterConfigFromSettings()));
    QTimer::singleShot(0, mainWindow, SLOT(changeWindowStatus()));

    mainAppHelper.changeMouseThread(inputEventThread);

    joypad_worker->startWorker();

    joypad_worker->moveToThread(inputEventThread);
    PadderCommon::mouseHelperObj.moveToThread(inputEventThread);
    inputEventThread->start(QThread::HighPriority);

    int app_result = antimicrox.exec();

    qInfo() << QObject::tr("Quitting Program");

    delete localServer;
    localServer = nullptr;

    if (!joypad_worker.isNull())
    {
        joypad_worker->deleteLater();
    }

    inputEventThread->quit();
    inputEventThread->wait();

    delete inputEventThread;
    inputEventThread = nullptr;

    delete joysticks;
    joysticks = nullptr;

    AntKeyMapper::getInstance()->deleteInstance();

#if defined(WITH_X11)

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        X11Extras::getInstance()->closeDisplay();
    }

#endif

    EventHandlerFactory::getInstance()->handler()->cleanup();
    EventHandlerFactory::getInstance()->deleteInstance();

    delete mainWindow;
    mainWindow = nullptr;

    delete appLogger;
    return app_result;
}
