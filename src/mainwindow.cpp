/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "messagehandler.h"
#include "inputdevice.h"
#include "autoprofileinfo.h"
#include "aboutdialog.h"
#include "commandlineutility.h"
#include "antimicrosettings.h"
//#include "autoprofilewatcher.h"
#include "joystick.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "joycontrolstickpushbutton.h"
#include "joytabwidget.h"
#include "joydpadbuttonwidget.h"
#include "joycontrolstickbuttonpushbutton.h"
#include "dpadpushbutton.h"
#include "joystickstatuswindow.h"
#include "qkeydisplaydialog.h"
#include "mainsettingsdialog.h"
#include "advancestickassignmentdialog.h"
#include "common.h"
#include "gamecontrollermappingdialog.h"
#include "calibration.h"
#include "xml/inputdevicexml.h"
#include "xml/joybuttonslotxml.h"

#if defined(WITH_X11)
    #include "autoprofileinfo.h"
    #include "autoprofilewatcher.h"
#endif

#include <QApplication>
#include <QHideEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QLocalServer>
#include <QTranslator>
#include <QAction>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include <QResource>
#include <QHashIterator>
#include <QMapIterator>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QFileInfo>
#include <QRegularExpression>

#include <SDL2/SDL_joystick.h>


#define CHECK_BATTERIES_MSEC 600000


MainWindow::MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                       CommandLineUtility *cmdutility, AntiMicroSettings *settings,
                       bool graphical, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon::fromTheme(QString::fromUtf8("antimicroX"), QIcon(":/images/antimicroX.png")));
    (QIcon::fromTheme(QString::fromUtf8("application_exit"),
                                              QIcon(":/icons/hicolor/16x16/actions/application_exit.png")));

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    ui->stackedWidget->setCurrentIndex(0);

    m_translator = nullptr;
    m_appTranslator = nullptr;
    m_cmdutility = cmdutility;
    m_graphical = graphical;
    m_settings = settings;

    ui->actionStick_Pad_Assign->setVisible(false);



    #if defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        this->appWatcher = new AutoProfileWatcher(settings, this);
        checkAutoProfileWatcherTimer();
    }
    else
    {
        this->appWatcher = nullptr;
        qDebug() << "appWatcher instance set to null pointer";
    }
    #endif

    signalDisconnect = false;
    showTrayIcon = !cmdutility->isTrayHidden() && graphical &&
                   !cmdutility->shouldListControllers() && !cmdutility->shouldMapController();

    m_joysticks = joysticks;

    if (showTrayIcon)
    {
        trayIconMenu = new QMenu(this);
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setContextMenu(trayIconMenu);

        connect(trayIconMenu, &QMenu::aboutToShow, this, &MainWindow::refreshTrayIconMenu);
        connect(trayIcon, &QSystemTrayIcon::activated,
                this, &MainWindow::trayIconClickAction);
    }

    // Look at flags and call setEnabled as desired; defaults to true.
    // Enabled status is used to specify whether errors in profile loading and
    // saving should be display in a window or written to stderr.
    if (graphical)
    {
        if (cmdutility->isHiddenRequested() && cmdutility->isTrayHidden())
        {
            setEnabled(false);
        }
    }
    else
    {
        setEnabled(false);
    }

    resize(settings->value("WindowSize", size()).toSize());
    move(settings->value("WindowPosition", pos()).toPoint());

    if (graphical)
    {
        aboutDialog = new AboutDialog(this);
    }
    else
    {
        aboutDialog = nullptr;
    }

    QMenu* menuPointer = ui->menuQuit;
    connect(ui->menuQuit, &QMenu::aboutToShow, this, [this, menuPointer] {
        mainMenuChange(menuPointer);
    });

    QMenu* menuPointerOptions = ui->menuOptions;
    connect(ui->menuOptions, &QMenu::aboutToShow, this, [this, menuPointerOptions] {
        mainMenuChange(menuPointerOptions);
    });

    connect(ui->actionKeyValue, &QAction::triggered, this, &MainWindow::openKeyCheckerDialog);
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionProperties, &QAction::triggered, this, &MainWindow::openJoystickStatusWindow);
    connect(ui->actionGitHubPage, &QAction::triggered, this, &MainWindow::openGitHubPage);
    connect(ui->actionIssues, &QAction::triggered, this, &MainWindow::openIssuesPage);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::openMainSettingsDialog);
    connect(ui->actionWiki, &QAction::triggered, this, &MainWindow::openWikiPage);
    connect(ui->actionCalibration, &QAction::triggered, this, &MainWindow::openCalibration);
    connect(ui->actionGameController_Mapping, &QAction::triggered, this, &MainWindow::openGameControllerMappingWindow);

    #if defined(WITH_X11)
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            connect(appWatcher, &AutoProfileWatcher::foundApplicableProfile, this, &MainWindow::autoprofileLoad);
        }
    #endif

    ui->uacPushButton->setVisible(false);


    QTimer *timer = new QTimer(this);

    connect(timer, &QTimer::timeout, [this]() {
        this->checkEachTenMinutesBattery(m_joysticks);
    });

    timer->start(CHECK_BATTERIES_MSEC);

}

MainWindow::~MainWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    qDebug() << "removing main window";

    if (trayIconMenu != nullptr) trayIconMenu->clear();

    installEventFilter(trayIconMenu);
    installEventFilter(trayIcon);

    if (trayIconMenu != nullptr) delete trayIconMenu;
    if (trayIcon != nullptr) delete trayIcon;

    delete ui;
}

void MainWindow::alterConfigFromSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_cmdutility->shouldListControllers())
    {
        m_graphical = false;
    }
    else if (m_cmdutility->hasProfile())
    {
        if (m_cmdutility->hasControllerNumber())
        {
            loadConfigFile(m_cmdutility->getProfileLocation(),
                           m_cmdutility->getControllerNumber());
        }
        else if (m_cmdutility->hasControllerID())
        {
            loadConfigFile(m_cmdutility->getProfileLocation(),
                           m_cmdutility->hasControllerID());
        }
        else
        {
            loadConfigFile(m_cmdutility->getProfileLocation());
        }
    }

    const QList<ControllerOptionsInfo> tempList = m_cmdutility->getControllerOptionsList();
    //unsigned int optionListSize = tempList->size();

    QListIterator<ControllerOptionsInfo> optionIter(tempList);
    while (optionIter.hasNext())
    {
        ControllerOptionsInfo temp = optionIter.next();
        if (temp.hasProfile())
        {
            if (temp.hasControllerNumber())
            {
                loadConfigFile(temp.getProfileLocation(),
                               temp.getControllerNumber());
            }
            else if (temp.hasControllerID())
            {
                loadConfigFile(temp.getProfileLocation(),
                               temp.hasControllerID());
            }
            else
            {
                loadConfigFile(temp.getProfileLocation());
            }
        }
        else if (temp.isUnloadRequested())
        {
            if (temp.hasControllerNumber())
            {
                unloadCurrentConfig(temp.getControllerNumber());
            }
            else if (temp.hasControllerID())
            {
                unloadCurrentConfig(temp.hasControllerID());
            }
            else
            {
                unloadCurrentConfig(0);
            }
        }

        if (temp.getStartSetNumber() > 0)
        {
            if (temp.hasControllerNumber())
            {
                changeStartSetNumber(temp.getJoyStartSetNumber(),
                                     temp.getControllerNumber());
            }
            else if (temp.hasControllerID())
            {
                changeStartSetNumber(temp.getJoyStartSetNumber(),
                                     temp.getControllerID());
            }
            else
            {
                changeStartSetNumber(temp.getJoyStartSetNumber());
            }
        }
    }
}

void MainWindow::controllerMapOpening()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_cmdutility->shouldMapController())
    {
        m_graphical = false;

        const QList<ControllerOptionsInfo> tempList = m_cmdutility->getControllerOptionsList();
        ControllerOptionsInfo temp = tempList.at(0);

        if (temp.hasControllerNumber())
        {

            int joypadIndex = m_cmdutility->getControllerNumber();

            qDebug() << "It was antimicroX --map controllerNumber";
            qDebug() << "controllerNumber: " << joypadIndex;

            selectControllerJoyTab(joypadIndex);
            openGameControllerMappingWindow(true);
        }
        else if (temp.hasControllerID())
        {
            QString joypadGUID = m_cmdutility->getControllerID();

            qDebug() << "It was antimicroX --map controllerID";
            qDebug() << "controllerID: " << joypadGUID;

            selectControllerJoyTab(joypadGUID);
            openGameControllerMappingWindow(true);
        }
        else
        {
            qDebug() << "Could not find a proper controller identifier. Exiting";

            Logger::LogInfo(tr("Could not find a proper controller identifier. "
                               "Exiting."));
            qApp->quit();
        }
    }
}


void MainWindow::fillButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    fillButtonsMap(m_joysticks);
}

void MainWindow::makeJoystickTabs()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

    // Make temporary QMap with devices inserted using the device index as the
    // key rather than joystick ID.
    QMap<SDL_JoystickID, InputDevice*> temp;
    QMapIterator<SDL_JoystickID, InputDevice*> iterTemp(*m_joysticks);

    while (iterTemp.hasNext())
    {
        iterTemp.next();

        InputDevice *joystick = iterTemp.value();
        temp.insert(joystick->getJoyNumber(), joystick);
    }

    QMapIterator<SDL_JoystickID, InputDevice*> iter(temp);

    while (iter.hasNext())
    {
        iter.next();

        InputDevice *joystick = iter.value();
        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, m_settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(tr("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
    }

    if (m_joysticks != nullptr)
    {
        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::fillButtonsID(InputDevice *joystick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));  // static_cast
    tabwidget->refreshButtons();
}

void MainWindow::fillButtonsMap(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

    // Make temporary QMap with devices inserted using the device index as the
    // key rather than joystick ID.
    QMap<SDL_JoystickID, InputDevice*> temp;
    QMapIterator<SDL_JoystickID, InputDevice*> iterTemp(*joysticks);

    while (iterTemp.hasNext())
    {
        iterTemp.next();

        InputDevice *joystick = iterTemp.value();
        temp.insert(joystick->getJoyNumber(), joystick);
    }

    QMapIterator<SDL_JoystickID, InputDevice*> iter(temp);

    while (iter.hasNext())
    {
        iter.next();

        InputDevice *joystick = iter.value();

        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, m_settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(tr("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
        tabwidget->refreshButtons();

        connect(tabwidget, &JoyTabWidget::namesDisplayChanged, this, [this, tabwidget](bool displayNames) {
            propogateNameDisplayStatus(tabwidget, displayNames);
        });

        connect(tabwidget, &JoyTabWidget::mappingUpdated, this, &MainWindow::propogateMappingUpdate);

        if (showTrayIcon)
        {
            connect(tabwidget, &JoyTabWidget::joystickConfigChanged, this, &MainWindow::populateTrayIcon);
        }
    }

    if (joysticks->size() > 0)
    {
        loadAppConfig();

        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }

    if (showTrayIcon)
    {
        populateTrayIcon();
        trayIcon->show();
    }

    ui->actionUpdate_Joysticks->setEnabled(true);
    ui->actionHide->setEnabled(true);
    ui->actionQuit->setEnabled(true);

}

// Intermediate slot to be used in Form Designer
void MainWindow::startJoystickRefresh()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->stackedWidget->setCurrentIndex(0);
    ui->actionUpdate_Joysticks->setEnabled(false);
    ui->actionHide->setEnabled(false);
    ui->actionQuit->setEnabled(false);
    removeJoyTabs();

    emit joystickRefreshRequested();
}

void MainWindow::populateTrayIcon()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(trayIconMenu, &QMenu::aboutToShow, this, &MainWindow::singleTrayProfileMenuShow);

    trayIconMenu->clear();
    profileActions.clear();

    closeAction = new QAction(tr("&Quit"), trayIconMenu);
    closeAction->setIcon(QIcon::fromTheme(QString::fromUtf8("application_exit"),
                                          QIcon(":/icons/hicolor/16x16/actions/application_exit.png")));

    connect(closeAction, &QAction::triggered, this, &MainWindow::quitProgram, Qt::DirectConnection);

    hideAction = new QAction(tr("&Hide"), trayIconMenu);
    hideAction->setIcon(QIcon::fromTheme(QString::fromUtf8("view_restore"),
                                         QIcon(":/icons/hicolor/16x16/actions/view_restore.png")));

    connect(hideAction, &QAction::triggered, this, &MainWindow::hideWindow);

    restoreAction = new QAction(tr("&Restore"), trayIconMenu);
    qDebug() << " Application theme has icon named view_fullscreen: " << QIcon::hasThemeIcon("view_fullscreen");
    restoreAction->setIcon(QIcon::fromTheme(QString::fromUtf8("view_fullscreen"),
                                            QIcon(":/icons/hicolor/16x16/actions/view_fullscreen.png")));

    connect(restoreAction, &QAction::triggered, this, &MainWindow::show);

    updateJoy = new QAction(tr("&Update Joysticks"), trayIconMenu);
    updateJoy->setIcon(QIcon::fromTheme(QString::fromUtf8("view_refresh"),
                                        QIcon(":/icons/hicolor/16x16/actions/view_refresh.png")));

    connect(updateJoy, &QAction::triggered, this, &MainWindow::startJoystickRefresh);

    int joystickCount = m_joysticks->size();

    qDebug() << "joystickCount: " << joystickCount << endl;

    QMap<QString,int> uniques = QMap<QString,int>();
    int counterUniques = 1;
    bool duplicatedGamepad = false;


    if (joystickCount > 0)
    {
        QMapIterator<SDL_JoystickID, InputDevice*> iter(*m_joysticks);
        bool useSingleList = m_settings->value("TrayProfileList", false).toBool();

        qDebug() << "TrayProfileList: " << useSingleList << endl;

        if (!useSingleList && (joystickCount == 1)) useSingleList = true;

        int i = 0;

        while (iter.hasNext())
        {
            iter.next();
            InputDevice *current = iter.value();

            QString joytabName = current->getSDLName();
            joytabName.append(" ")
                    .append(tr("(%1)")
                    .arg(current->getName()));

            qDebug() << "joytabName" << i << ": " << joytabName << endl;
            QMenu *joysticksubMenu = nullptr;

            if (!useSingleList) joysticksubMenu = trayIconMenu->addMenu(joytabName);


            JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast

            if (widget != nullptr)
            {
                QHash<int, QString> *configs = widget->recentConfigs();
                QHashIterator<int, QString> configIter(*configs);
                QList<QAction*> tempProfileList;

                while (configIter.hasNext())
                {
                    configIter.next();
                    QAction *newaction = nullptr;

                    if (joysticksubMenu != nullptr)
                    {
                        newaction = new QAction(configIter.value(), joysticksubMenu);
                    }
                    else
                    {
                        newaction = new QAction(configIter.value(), trayIconMenu);
                    }

                    newaction->setCheckable(true);
                    newaction->setChecked(false);

                    if (uniques.contains(current->getGUIDString()) &&
                            joystickCount < uniques[current->getGUIDString()])
                    {
                        ++uniques[current->getGUIDString()];
                        duplicatedGamepad = true;
                    }
                    else if (joystickCount < uniques[current->getGUIDString()])
                    {
                        uniques.insert(current->getGUIDString(), counterUniques);
                    }

                    int resultDuplicated = 0;
                    if (duplicatedGamepad) resultDuplicated = uniques[current->getGUIDString()];

                    current->setCounterUniques(resultDuplicated);
                    QString identifier = current->getStringIdentifier();
                    qDebug() << "current identifier: " << current->getStringIdentifier() << endl;
                    widget->convToUniqueIDControllerGroupSett(m_settings, QString("Controller%1LastSelected").arg(current->getGUIDString()), QString("Controller%1LastSelected").arg(current->getUniqueIDString()));
                    QString controlEntryLastSelected = QString("Controller%1LastSelected").arg(identifier);

                    qDebug() << "controlEntryLastSelected: " << controlEntryLastSelected << endl;

                    duplicatedGamepad = false;

                    QString contrFile = m_settings->value(controlEntryLastSelected).toString();

                    QFileInfo fileInfo(contrFile);

                    qDebug() << "controlEntryLastSelected in config file: " << contrFile << endl;

                    qDebug() << "fileInfo.exists(): " << fileInfo.exists() << endl;
                    qDebug() << "fileInfo.size(): " << fileInfo.size() << endl;
                    qDebug() << "fileInfo.permissions(): " << fileInfo.permissions() << endl;


                    if ((configIter.value() == fileInfo.baseName()) || (configIter.value() == widget->getCurrentConfigName()))
                    {
                        qDebug() << "fileInfo.baseName(): " << fileInfo.baseName() << endl;
                        qDebug() << "widget->getCurrentConfigName(): " << widget->getCurrentConfigName() << endl;
                        newaction->setChecked(true);
                    }

                    QHash<QString, QVariant> tempmap;
                    qDebug() << "insert " << QString::number(i) << ": " << configIter.key() << endl;
                    tempmap.insert(QString::number(i), QVariant (configIter.key()));
                    QVariant tempvar (tempmap);
                    newaction->setData(tempvar);

                    connect(newaction, &QAction::triggered, this, [this, newaction](bool checked) {
                        profileTrayActionTriggered(newaction, checked);
                    });

                    if (useSingleList)
                    {
                        qDebug() << "useSingleList" << endl;
                        tempProfileList.append(newaction);
                    }
                    else
                    {
                        qDebug() << "doesn't useSingleList" << endl;
                        joysticksubMenu->addAction(newaction);
                    }
                }

                delete configs;
                configs = nullptr;

                QAction *newaction = nullptr;

                if (joysticksubMenu != nullptr)
                {
                    qDebug() << "joysticksubmenu exists" << endl;
                    newaction = new QAction(tr("Open File"), joysticksubMenu);
                }
                else
                {
                    qDebug() << "created action open file for tray" << endl;
                    newaction = new QAction(tr("Open File"), trayIconMenu);
                }

                newaction->setIcon(QIcon::fromTheme(QString::fromUtf8("document_open"),
                                                    QIcon(":/icons/hicolor/16x16/actions/document_open.png")));

                connect(newaction, &QAction::triggered, widget, &JoyTabWidget::openConfigFileDialog);

                if (useSingleList)
                {
                    qDebug() << "usesinglelist" << endl;
                    QAction *titleAction = new QAction(joytabName, trayIconMenu);
                    titleAction->setCheckable(false);

                    QFont actionFont = titleAction->font();
                    actionFont.setBold(true);
                    titleAction->setFont(actionFont);

                    trayIconMenu->addAction(titleAction);
                    trayIconMenu->addActions(tempProfileList);
                    trayIconMenu->addAction(newaction);

                    profileActions.insert(i, tempProfileList);
                    qDebug() << "inserted profile action " << i << ": " << tempProfileList << endl;

                    if (iter.hasNext())
                    {
                        trayIconMenu->addSeparator();
                    }
                }
                else
                {
                    joysticksubMenu->addAction(newaction);
                    connect(joysticksubMenu, &QMenu::aboutToShow, this, [this, joysticksubMenu] {
                        joystickTrayShow(joysticksubMenu);
                    });
                }

                i++;
            }
        }

        if (useSingleList)
        {
            connect(trayIconMenu, &QMenu::aboutToShow, this, &MainWindow::singleTrayProfileMenuShow);
        }

        trayIconMenu->addSeparator();
    }

    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(updateJoy);
    trayIconMenu->addAction(closeAction);

    QIcon icon = QIcon::fromTheme("antimicroX", QIcon(":/images/antimicroX_trayicon.png"));
    trayIcon->setIcon(icon);
    trayIcon->setContextMenu(trayIconMenu);

    qDebug() << "end of MainWindow::populateTrayIcon function" << endl;
}

void MainWindow::quitProgram()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool discard = true;
#ifdef WITH_X11
    AutoProfileWatcher::getAutoProfileWatcherInstance()->disconnectWindowTimer();
#endif

    for (int i = 0; (i < ui->tabWidget->count()) && discard; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        discard = tab->discardUnsavedProfileChanges();
    }

    if (discard) qApp->quit();

}

void MainWindow::refreshTrayIconMenu()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (this->isHidden())
    {
        hideAction->setEnabled(false);
        restoreAction->setEnabled(true);
    }
    else
    {
        hideAction->setEnabled(true);
        restoreAction->setEnabled(false);
    }
}

void MainWindow::trayIconClickAction(QSystemTrayIcon::ActivationReason reason)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (reason == QSystemTrayIcon::Trigger)
    {
        if (this->isHidden())
        {
            this->show();
        }
        else
        {
            this->hideWindow();
        }
    }
}

void MainWindow::mainMenuChange(QMenu* tempMenu)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (tempMenu == ui->menuQuit)
    {
        if (showTrayIcon)
        {
            ui->actionHide->setEnabled(true);
        }
        else
        {
            ui->actionHide->setEnabled(false);
        }
    }
}

void MainWindow::saveAppConfig()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_joysticks->size() > 0)
    {
        JoyTabWidget *temptabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(0)); // static_cast
        m_settings->setValue("DisplayNames",
            temptabwidget->isDisplayingNames() ? "1" : "0");

        m_settings->beginGroup("Controllers");
        QStringList tempIdentifierHolder = QStringList();

        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            bool prepareSave = true;

            JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
            InputDevice *device = tabwidget->getJoystick();

            // Do not allow multi-controller adapters to overwrite each
            // others recent config file list. Use first controller
            // detected to save recent config list. Flag controller string
            // afterwards.
            if (!device->getStringIdentifier().isEmpty())
            {
                if (tempIdentifierHolder.contains(device->getStringIdentifier()))
                {
                    prepareSave = false;
                }
                else
                {
                    tempIdentifierHolder.append(device->getStringIdentifier());
                }
            }

            if (prepareSave)
            {
                tabwidget->saveSettings();
            }
        }

        m_settings->endGroup();
    }

    m_settings->setValue("WindowSize", size());
    m_settings->setValue("WindowPosition", pos());
}

void MainWindow::loadAppConfig(bool forceRefresh)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        tabwidget->loadSettings(forceRefresh);
    }
}

void MainWindow::disableFlashActions()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        QList<JoyButtonWidget*> list = ui->tabWidget->widget(i)->findChildren<JoyButtonWidget*>();
        QListIterator<JoyButtonWidget*> iter(list);
        while (iter.hasNext())
        {
            JoyButtonWidget *buttonWidget = iter.next();
            buttonWidget->disableFlashes();
        }

        QList<JoyAxisWidget*> list2 = ui->tabWidget->widget(i)->findChildren<JoyAxisWidget*>();
        QListIterator<JoyAxisWidget*> iter2(list2);
        while (iter2.hasNext())
        {
            JoyAxisWidget *axisWidget = iter2.next();
            axisWidget->disableFlashes();
        }

        QList<JoyControlStickPushButton*> list3 = ui->tabWidget->widget(i)->findChildren<JoyControlStickPushButton*>();
        QListIterator<JoyControlStickPushButton*> iter3(list3);
        while (iter3.hasNext())
        {
            JoyControlStickPushButton *stickWidget = iter3.next();
            stickWidget->disableFlashes();
        }

        QList<JoyDPadButtonWidget*> list4 = ui->tabWidget->widget(i)->findChildren<JoyDPadButtonWidget*>();
        QListIterator<JoyDPadButtonWidget*> iter4(list4);
        while (iter4.hasNext())
        {
            JoyDPadButtonWidget *dpadWidget = iter4.next();
            dpadWidget->disableFlashes();
        }

        QList<JoyControlStickButtonPushButton*> list6 = ui->tabWidget->widget(i)->findChildren<JoyControlStickButtonPushButton*>();
        QListIterator<JoyControlStickButtonPushButton*> iter6(list6);
        while (iter6.hasNext())
        {
            JoyControlStickButtonPushButton *stickButtonWidget = iter6.next();
            stickButtonWidget->disableFlashes();
        }

        QList<DPadPushButton*> list7 = ui->tabWidget->widget(i)->findChildren<DPadPushButton*>();
        QListIterator<DPadPushButton*> iter7(list7);
        while (iter7.hasNext())
        {
            DPadPushButton *dpadWidget = iter7.next();
            dpadWidget->disableFlashes();
        }

        JoyTabWidget *tabWidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        ui->tabWidget->disableFlashes(tabWidget->getJoystick());
    }
}

void MainWindow::enableFlashActions()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        QList<JoyButtonWidget*> list = ui->tabWidget->widget(i)->findChildren<JoyButtonWidget*>();
        QListIterator<JoyButtonWidget*> iter(list);
        while (iter.hasNext())
        {
            JoyButtonWidget *buttonWidget = iter.next();
            buttonWidget->enableFlashes();
            buttonWidget->tryFlash();
        }

        QList<JoyAxisWidget*> list2 = ui->tabWidget->widget(i)->findChildren<JoyAxisWidget*>();
        QListIterator<JoyAxisWidget*> iter2(list2);
        while (iter2.hasNext())
        {
            JoyAxisWidget *axisWidget = iter2.next();
            axisWidget->enableFlashes();
            axisWidget->tryFlash();
        }

        QList<JoyControlStickPushButton*> list3 = ui->tabWidget->widget(i)->findChildren<JoyControlStickPushButton*>();
        QListIterator<JoyControlStickPushButton*> iter3(list3);
        while (iter3.hasNext())
        {
            JoyControlStickPushButton *stickWidget = iter3.next();
            stickWidget->enableFlashes();
            stickWidget->tryFlash();
        }

        QList<JoyDPadButtonWidget*> list4 = ui->tabWidget->widget(i)->findChildren<JoyDPadButtonWidget*>();
        QListIterator<JoyDPadButtonWidget*> iter4(list4);
        while (iter4.hasNext())
        {
            JoyDPadButtonWidget *dpadWidget = iter4.next();
            dpadWidget->enableFlashes();
            dpadWidget->tryFlash();
        }

        QList<JoyControlStickButtonPushButton*> list6 = ui->tabWidget->widget(i)->findChildren<JoyControlStickButtonPushButton*>();
        QListIterator<JoyControlStickButtonPushButton*> iter6(list6);
        while (iter6.hasNext())
        {
            JoyControlStickButtonPushButton *stickButtonWidget = iter6.next();
            stickButtonWidget->enableFlashes();
            stickButtonWidget->tryFlash();
        }

        QList<DPadPushButton*> list7 = ui->tabWidget->widget(i)->findChildren<DPadPushButton*>();
        QListIterator<DPadPushButton*> iter7(list7);
        while (iter7.hasNext())
        {
            DPadPushButton *dpadWidget = iter7.next();
            dpadWidget->enableFlashes();
            dpadWidget->tryFlash();
        }

        JoyTabWidget *tabWidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        ui->tabWidget->enableFlashes(tabWidget->getJoystick());
    }
}

// Intermediate slot used in Design mode
void MainWindow::hideWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disableFlashActions();
    signalDisconnect = true;
    hide();
}

void MainWindow::joystickTrayShow(QMenu* tempmenu)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<QAction*> menuactions = tempmenu->actions();
    QListIterator<QAction*> listiter (menuactions);
    while (listiter.hasNext())
    {
        QAction *action = listiter.next();
        action->setChecked(false);

        QHash<QString, QVariant> tempmap = action->data().toHash();
        QHashIterator<QString, QVariant> iter(tempmap);
        while (iter.hasNext())
        {
            iter.next();
            int joyindex = iter.key().toInt();
            int configindex = iter.value().toInt();
            JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex)); // static_cast

            if (configindex == widget->getCurrentConfigIndex())
            {
                action->setChecked(true);

                if (widget->getJoystick()->isDeviceEdited())
                {
                    action->setIcon(QIcon::fromTheme(QString::fromUtf8("document_save_as"),
                                                     QIcon(":/icons/hicolor/16x16/actions/document_save_as.png")));

                }
                else if (!action->icon().isNull())
                {
                    action->setIcon(QIcon());
                }
            }
            else if (!action->icon().isNull())
            {
                action->setIcon(QIcon());
            }

            if (action->text() != widget->getConfigName(configindex))
            {
                action->setText(widget->getConfigName(configindex));
            }
        }
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Check if hideEvent has been processed
    if (signalDisconnect && isVisible())
    {
        // Restore flashing buttons
        enableFlashActions();
        signalDisconnect = false;

        // Only needed if hidden with the system tray enabled
        if (showTrayIcon)
        {
            if (isMinimized())
            {
                if (isMaximized())
                {
                    showMaximized();
                }
                else
                {
                    showNormal();
                }

                activateWindow();
                raise();
            }
        }
    }

    QMainWindow::showEvent(event);

}

void MainWindow::changeEvent(QEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *e = static_cast<QWindowStateChangeEvent*>(event);
        if (e->oldState() != Qt::WindowMinimized && isMinimized())
        {
            bool minimizeToTaskbar = m_settings->value("MinimizeToTaskbar", false).toBool();
            if (QSystemTrayIcon::isSystemTrayAvailable() && showTrayIcon && !minimizeToTaskbar)
            {
                this->hideWindow();
            }
            else
            {
                disableFlashActions();
                signalDisconnect = true;
            }
        }
    }
    else if (event->type() == QEvent::LanguageChange)
    {
        retranslateUi();
    }

    QMainWindow::changeEvent(event);
}


void MainWindow::openAboutDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    aboutDialog->show();
}


void MainWindow::loadConfigFile(QString fileLocation, int joystickIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joystickIndex > 0) && m_joysticks->contains(joystickIndex - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joystickIndex-1)); // static_cast
        if (widget != nullptr)
        {
            widget->loadConfigFile(fileLocation);
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
            if (widget != nullptr)
            {
                widget->loadConfigFile(fileLocation);
            }
        }
    }
}


void MainWindow::loadConfigFile(QString fileLocation, QString controllerID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab != nullptr)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getStringIdentifier())
                {
                    tab->loadConfigFile(fileLocation);
                }
            }
        }
    }
}


void MainWindow::removeJoyTabs()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int oldtabcount = ui->tabWidget->count();

    for (int i = oldtabcount-1; i >= 0; i--)
    {
        QWidget *tab = ui->tabWidget->widget(i);
        delete tab;
        tab = nullptr;
    }

    ui->tabWidget->clear();
}


void MainWindow::handleInstanceDisconnect()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_settings->sync();
    loadAppConfig(true);
}


void MainWindow::openJoystickStatusWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick != nullptr)
        {
            JoystickStatusWindow *dialog = new JoystickStatusWindow(joystick, this);
            dialog->show();
        }
    }
}


void MainWindow::openKeyCheckerDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QKeyDisplayDialog *dialog = new QKeyDisplayDialog(this);
    dialog->show();
}


void MainWindow::openGitHubPage()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QDesktopServices::openUrl(QUrl(PadderCommon::githubProjectPage));
}


void MainWindow::openIssuesPage()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QDesktopServices::openUrl(QUrl(PadderCommon::githubIssuesPage));
}


void MainWindow::openWikiPage()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QDesktopServices::openUrl(QUrl(PadderCommon::wikiPage));
}


void MainWindow::openCalibration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_joysticks->isEmpty()) {

        QMessageBox::information(this, tr("Calibration couldn't be opened"), tr("You must connect at least one controller to open the window"));

    } else {

        int index = ui->tabWidget->currentIndex();
        if (index >= 0)
        {
            JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
            InputDevice *joystick = joyTab->getJoystick();

            if (joystick != nullptr)
            {
                QPointer<Calibration> calibration = new Calibration(joystick);
                calibration.data()->show();

                if (calibration.isNull())
                    calibration.clear();
            }
        }
    }
}


void MainWindow::unloadCurrentConfig(int joystickIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joystickIndex > 0) && m_joysticks->contains(joystickIndex - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*> (ui->tabWidget->widget(joystickIndex - 1)); // static_cast
        if (widget != nullptr)
        {
            widget->unloadConfig();
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i = 0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = qobject_cast<JoyTabWidget*> (ui->tabWidget->widget(i));  // static_cast
            if (widget != nullptr)
            {
                widget->unloadConfig();
            }
        }
    }
}


void MainWindow::unloadCurrentConfig(QString controllerID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab != nullptr)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getStringIdentifier())
                {
                    tab->unloadConfig();
                }
            }
        }
    }
}


void MainWindow::propogateNameDisplayStatus(JoyTabWidget* tabwidget, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

     for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast
        if ((tab != nullptr) && (tab != tabwidget))
        {
            if (tab->isDisplayingNames() != displayNames)
            {
                tab->changeNameDisplay(displayNames);
            }
        }
    }
}


void MainWindow::changeStartSetNumber(int startSetNumber, QString controllerID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab != nullptr)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getStringIdentifier())
                {
                    tab->changeCurrentSet(startSetNumber);
                }
            }
        }
    }
}


void MainWindow::changeStartSetNumber(int startSetNumber, int joystickIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joystickIndex > 0) && m_joysticks->contains(joystickIndex - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joystickIndex - 1));  // static_cast
        if (widget != nullptr)
        {
            widget->changeCurrentSet(startSetNumber);
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast
            if (widget != nullptr)
            {
                widget->changeCurrentSet(startSetNumber);
            }
        }
    }
}


/**
 * @brief Build list of current input devices and pass it to settings dialog
 *     instance. Open Settings dialog.
 */

void MainWindow::openMainSettingsDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<InputDevice*> *devices = new QList<InputDevice*>(m_joysticks->values());
    MainSettingsDialog *dialog = new MainSettingsDialog(m_settings, devices, this);
    connect(dialog, &MainSettingsDialog::changeLanguage, this, &MainWindow::changeLanguage);

    if (appWatcher != nullptr)
    {
#if defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    connect(dialog, &MainSettingsDialog::accepted, appWatcher, &AutoProfileWatcher::syncProfileAssignment);
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkAutoProfileWatcherTimer);
    connect(dialog, &MainSettingsDialog::rejected, this, &MainWindow::checkAutoProfileWatcherTimer);
    appWatcher->stopTimer();
    qDebug() << "Stopping appWatcher in openMainSettingsDialog";
    }
#endif
    }

    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::populateTrayIcon);
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkHideEmptyOption);

    dialog->show();
}


/**
 * @brief Change language used by the application.
 * @param Language code
 */
void MainWindow::changeLanguage(QString language)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((m_translator != nullptr) && (m_appTranslator != nullptr))
    {
        PadderCommon::reloadTranslations(m_translator, m_appTranslator, language);
    }
}


/**
 * @brief Check if the program should really quit or if it should
 *     be minimized.
 * @param QCloseEvent
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool closeToTray = m_settings->value("CloseToTray", false).toBool();
    if (closeToTray && QSystemTrayIcon::isSystemTrayAvailable() && showTrayIcon)
    {
        this->hideWindow();
    }
    else
    {
        qApp->quit();
    }

    QMainWindow::closeEvent(event);
}


/**
 * @brief Show abstracted controller dialog for use in SDL 1.2. No longer
 *     used for versions of the program running SDL 2. In SDL 2,
 *     the Game Controller API is being used instead.
 */
void MainWindow::showStickAssignmentDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
        Joystick *joystick = qobject_cast<Joystick*>(joyTab->getJoystick()); // static_cast

        AdvanceStickAssignmentDialog *dialog = new AdvanceStickAssignmentDialog(joystick, this);
        connect(dialog, &AdvanceStickAssignmentDialog::finished, joyTab, &JoyTabWidget::fillButtons);
        dialog->show();
    }
}


/**
 * @brief Display a version of the tray menu that shows all recent profiles for
 *    all controllers in one list.
 */
void MainWindow::singleTrayProfileMenuShow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!getProfileActions().isEmpty())
    {
        QMapIterator<int, QList<QAction*> > mapIter(getProfileActions());
        while (mapIter.hasNext())
        {
            mapIter.next();
            QList<QAction*> menuactions = mapIter.value();
            QListIterator<QAction*> listiter (menuactions);
            while (listiter.hasNext())
            {
                QAction *action = listiter.next();
                action->setChecked(false);

                QHash<QString, QVariant> tempmap = action->data().toHash();
                QHashIterator<QString, QVariant> iter(tempmap);
                while (iter.hasNext())
                {
                    iter.next();
                    int joyindex = iter.key().toInt();
                    int configindex = iter.value().toInt();
                    JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex)); // static_cast

                    if (configindex == widget->getCurrentConfigIndex())
                    {
                        action->setChecked(true);

                        if (widget->getJoystick()->isDeviceEdited())
                        {
                            action->setIcon(QIcon::fromTheme(QString::fromUtf8("document_save_as"),
                                                             QIcon(":/icons/hicolor/16x16/actions/document_save_as.png")));
                        }
                        else if (!action->icon().isNull())
                        {
                            action->setIcon(QIcon());
                        }
                    }
                    else if (!action->icon().isNull())
                    {
                        action->setIcon(QIcon());
                    }

                    if (action->text() != widget->getConfigName(configindex))
                    {
                        action->setText(widget->getConfigName(configindex));
                    }
                }
            }
        }
    }
}


void MainWindow::profileTrayActionTriggered(QAction *action, bool checked)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Obtaining the selected config
    QHash<QString, QVariant> tempmap = action->data().toHash();
    QHashIterator<QString, QVariant> iter(tempmap);

    while (iter.hasNext())
    {
        iter.next();

        // Fetching indicies and tab associated with the current joypad
        int joyindex = iter.key().toInt();
        int configindex = iter.value().toInt();
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex)); // static_cast

        // Checking if the selected config has been disabled by the change (action->isChecked() represents the state of the checkbox AFTER the click)
        if (!checked)
        {
            // It has - disabling - the 0th config is the new/'null' config
            widget->setCurrentConfig(0);
        }
        else
        {
            // It hasn't - enabling - note that setting this causes the menu to be updated
            widget->setCurrentConfig(configindex);
        }
    }
}

void MainWindow::checkHideEmptyOption()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        if (tab != nullptr)
        {
            tab->checkHideEmptyOption();
        }
    }
}


void MainWindow::openGameControllerMappingWindow(bool openAsMain)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick != nullptr)
        {
            GameControllerMappingDialog *dialog = new GameControllerMappingDialog(joystick, m_settings, this);

            if (openAsMain)
            {
                dialog->setParent(nullptr);
                dialog->setWindowFlags(Qt::Window);
                connect(dialog, &GameControllerMappingDialog::finished, qApp, &QApplication::quit);
            }
            else
            {
                connect(dialog, &GameControllerMappingDialog::mappingUpdate, this, &MainWindow::propogateMappingUpdate);
            }

            dialog->show();
        }
    }
    else if (openAsMain)
    {
        Logger::LogInfo(tr("Could not find controller. Exiting."));
        qApp->quit();
    }
}

void MainWindow::propogateMappingUpdate(QString mapping, InputDevice *device)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit mappingUpdated(mapping, device);
}

void MainWindow::testMappingUpdateNow(int index, InputDevice *device)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWidget *tab = ui->tabWidget->widget(index);
    if (tab != nullptr)
    {
        ui->tabWidget->removeTab(index);
        delete tab;
        tab = nullptr;
    }

    JoyTabWidget *tabwidget = new JoyTabWidget(device, m_settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
    ui->tabWidget->insertTab(index, tabwidget, joytabName);
    tabwidget->refreshButtons();
    ui->tabWidget->setCurrentIndex(index);

    connect(tabwidget, &JoyTabWidget::namesDisplayChanged, this, [this, tabwidget](bool displayNames) {
        propogateNameDisplayStatus(tabwidget, displayNames);
    });

    connect(tabwidget, &JoyTabWidget::mappingUpdated, this, &MainWindow::propogateMappingUpdate);

    if (showTrayIcon)
    {
        connect(tabwidget, &JoyTabWidget::joystickConfigChanged, this, &MainWindow::populateTrayIcon);
        populateTrayIcon();
    }
}

void MainWindow::removeJoyTab(SDL_JoystickID deviceID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;
    for (int i=0; (i < ui->tabWidget->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        if ((tab != nullptr) && (deviceID == tab->getJoystick()->getSDLJoystickID()))
        {
            // Save most recent profile list to settings before removing tab.
            tab->saveDeviceSettings();

            // Remove flash event connections between buttons and
            // the tab before deleting tab.
            ui->tabWidget->disableFlashes(tab->getJoystick());
            ui->tabWidget->removeTab(i);
            QMetaObject::invokeMethod(tab->getJoystick(), "finalRemoval");
            delete tab;
            tab = nullptr;
            found = true;
        }
    }

    // Refresh tab text to reflect new index values.
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast
        if (tab != nullptr)
        {
            InputDevice *device = tab->getJoystick();
            QString joytabName = device->getSDLName();
            joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
            ui->tabWidget->setTabText(i, joytabName);
        }
    }

    if (showTrayIcon) populateTrayIcon();

    if (ui->tabWidget->count() == 0)
    {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::addJoyTab(InputDevice *device)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyTabWidget *tabwidget = new JoyTabWidget(device, m_settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
    ui->tabWidget->addTab(tabwidget, joytabName);
    tabwidget->loadDeviceSettings();
    tabwidget->refreshButtons();

    // Refresh tab text to reflect new index values.
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        if (tab != nullptr)
        {
            InputDevice *device_in_loop = tab->getJoystick();
            QString joytabName_in_loop = device_in_loop->getSDLName();
            joytabName_in_loop.append(" ").append(tr("(%1)").arg(device_in_loop->getName()));
            ui->tabWidget->setTabText(i, joytabName_in_loop);
        }
    }

    connect(tabwidget, &JoyTabWidget::namesDisplayChanged, this, [this, tabwidget](bool displayNames) {
        propogateNameDisplayStatus(tabwidget, displayNames);
    });

    connect(tabwidget, &JoyTabWidget::mappingUpdated, this, &MainWindow::propogateMappingUpdate);
    if (showTrayIcon)
    {
        connect(tabwidget, &JoyTabWidget::joystickConfigChanged, this, &MainWindow::populateTrayIcon);
        populateTrayIcon();
    }

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::autoprofileLoad(AutoProfileInfo *info)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

  if( info != nullptr ) {
    Logger::LogDebug(QObject::tr("Auto-switching to profile \"%1\".").
                     arg(info->getProfileLocation()));
  } else {
    Logger::LogError(QObject::tr("Auto-switching to nullptr profile!"));
  }
  
#if defined(WITH_X11)

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {

    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast

        if (widget != nullptr)
        {
            //if (info->getGUID() == "all")
            if (info->getUniqueID() == "all")
            {
                // If the all option for a Default profile was found,
                // first check for controller specific associations. If one exists,
                // skip changing the profile on the controller. A later call will
                // be used to switch the profile for that controller.
                QList<AutoProfileInfo*> *customs = appWatcher->getCustomDefaults();
                bool found = false;
                QListIterator<AutoProfileInfo*> iter(*customs);

                while (iter.hasNext())
                {
                    AutoProfileInfo *tempinfo = iter.next();

//                    if (tempinfo->getGUID() == widget->getJoystick()->getGUIDString() &&
                     if (tempinfo->getUniqueID() == widget->getJoystick()->getUniqueIDString() &&
                        info->isCurrentDefault())
                    {
                        found = true;
                        iter.toBack();
                        qDebug() << "autoProfileInfo has the same GUID as GUID of joystick and the autoProfile is default. Found = true.";
                    }
                }

                delete customs;
                customs = nullptr;

                // Check if profile has already been switched for a particular
                // controller.
                if (!found)
                {
//                    QString tempguid = widget->getJoystick()->getGUIDString();
//                    if (appWatcher->isGUIDLocked(tempguid))

                    QString tempguid = widget->getJoystick()->getUniqueIDString();

                    if (appWatcher->isUniqueIDLocked(tempguid))
                    {
                        found = true;
                        qDebug() << "GUID is locked in appWatcher. Found = true.";
                    }
                }

                if (!found)
                {
                    // If the profile location is empty, assume
                    // that an empty profile should get loaded.
                    if (info->getProfileLocation().isEmpty())
                    {
                        widget->setCurrentConfig(0);
                        qDebug() << "profile location is empty. setCurrentConfig(0)";
                    }
                    else
                    {
                        widget->loadConfigFile(info->getProfileLocation());
                        qDebug() << "loaded config file for current AutoLoadInfo";
                    }
                }
            }
            // else if (info->getGUID() == widget->getJoystick()->getStringIdentifier())
            else if (info->getUniqueID() == widget->getJoystick()->getStringIdentifier())
            {
                // qDebug() << "GUID of AutoProfileInfo: " << info->getGUID() << " == string identifier of AutoProfileInfo: " << widget->getJoystick()->getStringIdentifier();
                qDebug() << "GUID of AutoProfileInfo: " << info->getUniqueID() << " == string identifier of AutoProfileInfo: " << widget->getJoystick()->getStringIdentifier();

                if (info->getProfileLocation().isEmpty())
                {
                    qDebug() << "profile location of AutoProfileInfo is empty. Set first config";
                    widget->setCurrentConfig(0);
                }
                else
                {
                    qDebug() << "load config file for AutoProfileInfo";
                    widget->loadConfigFile(info->getProfileLocation());
                }
            }
        }
    }

    }

#endif
}

void MainWindow::checkAutoProfileWatcherTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

#if defined(WITH_X11)

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        QString autoProfileActive = m_settings->value("AutoProfiles/AutoProfilesActive", "0").toString();
        if (autoProfileActive == "1")
        {
            appWatcher->startTimer();
            qDebug() << "Started timer for appWatcher";
        }
        else
        {
            appWatcher->stopTimer();
            qDebug() << "Stopped timer for appWatcher";
        }
    }
#endif
}

/**
 * @brief TODO: Check if method is save to remove.
 */
void MainWindow::updateMenuOptions()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index));  // static_cast
        InputDevice *joystick = joyTab->getJoystick();

        if (qobject_cast<GameController*>(joystick) != nullptr)
        {
            ui->actionStick_Pad_Assign->setEnabled(false);
        }
        else
        {
            ui->actionStick_Pad_Assign->setEnabled(true);
        }
    }
}

void MainWindow::showBatteryLevel(SDL_JoystickPowerLevel powerLevSDL, QString batteryLev, QString percent, InputDevice* device)
{
    if (SDL_JoystickCurrentPowerLevel(device->getJoyHandle()) == powerLevSDL)
      {
        QResource batteryFile(":/images/battery-low-level.png");
        QPixmap pm(30,30);
        pm.load(batteryFile.absoluteFilePath());

        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("%1 battery").arg(batteryLev));
        msgBox.setIconPixmap(pm);
        msgBox.setText(tr("Battery level is less than %1").arg(percent));
        msgBox.setInformativeText(tr("Device number: %1\nDevice name: %2").arg(device->getRealJoyNumber()).arg(device->getSDLName()));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

/**
 * @brief Select appropriate tab with the specified index.
 * @param Index of appropriate tab.
 */
void MainWindow::selectControllerJoyTab(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((index > 0) && m_joysticks->contains(index - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*> (ui->tabWidget->widget(index - 1));  // static_cast
        if (widget != nullptr)
        {
            qDebug() << "JoyTabWidget was not a null pointer in selectControllerJoyTab of index";
            ui->tabWidget->setCurrentIndex(index - 1);
        }
        else
        {
            qDebug() << "JoyTabWidget was a NULL POINTER in selectControllerJoyTab of index";
        }
    }
}

/**
 * @brief Select appropriate tab that has a device with the specified GUID.
 * @param GUID of joystick device.
 */
void MainWindow::selectControllerJoyTab(QString GUID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!GUID.isEmpty())
    {
        InputDevice *device = nullptr;
        QMapIterator<SDL_JoystickID, InputDevice*> deviceIter(*m_joysticks);

        while (deviceIter.hasNext())
        {
            deviceIter.next();
            InputDevice *tempDevice = deviceIter.value();

            if (tempDevice && (GUID == tempDevice->getStringIdentifier()))
            {
                device = tempDevice;
                deviceIter.toBack();
            }
        }

        if (device != nullptr)
        {
                qDebug() << "InputDevice was not a null pointer in selectControllerJoyTab of GUID";

            ui->tabWidget->setCurrentIndex(device->getJoyNumber());
        }
        else
        {
                qDebug() << "InputDevice was a NULL POINTER in selectControllerJoyTab of GUID";
        }
    }
}

void MainWindow::changeWindowStatus()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Check flags to see if user requested for the main window and the tray icon
    // to not be displayed.

    if (m_graphical)
    {
        bool launchInTraySetting = m_settings->runtimeValue("LaunchInTray", false).toBool();
        if (!m_cmdutility->isHiddenRequested() &&
            (!launchInTraySetting || !QSystemTrayIcon::isSystemTrayAvailable()))
        {
            show();
        }
        else if (m_cmdutility->isHiddenRequested() && m_cmdutility->isTrayHidden())
        {
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();

            setEnabled(false); // Should already be disabled. Do it again just to be sure.
        }
        else if (m_cmdutility->isHiddenRequested() || launchInTraySetting)
        {
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();

            setEnabled(true);
        }
    }
}

bool MainWindow::getGraphicalStatus()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_graphical;
}

void MainWindow::setTranslator(QTranslator *translator)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_translator = translator;
}

QTranslator* MainWindow::getTranslator() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_translator;
}

void MainWindow::setAppTranslator(QTranslator *translator)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_appTranslator = translator;
}

QTranslator* MainWindow::getAppTranslator() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_appTranslator;
}

void MainWindow::retranslateUi()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->retranslateUi(this);
}

void MainWindow::refreshTabHelperThreads()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (widget != nullptr)
        {
            widget->refreshHelperThread();
        }
    }
}

QMap<int, QList<QAction*> > const& MainWindow::getProfileActions() {

    return profileActions;
}


bool MainWindow::isKeypadUnlocked()
{
    if (m_settings == nullptr) return false;

    return m_settings->value("AttachNumKeypad", false).toBool();
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Hide && (obj != nullptr)) {
        obj->deleteLater();
    }

    return false;
}


void MainWindow::convertGUIDtoUniqueID(InputDevice* currentDevice, QString controlEntryLastSelectedGUID)
{
        int exec = QMessageBox::information(this, tr("Reading old profile"), tr("This profile uses controllers' GUID numbers. Would you like to change GUID numbers to UniqueID in this file for use in identical gamecontrollers? Such old file cannot be loaded in antimicroX since version 2.25"), QMessageBox::Yes, QMessageBox::No);

        switch (exec)
        {
            case QMessageBox::Yes:

                QFile data(m_settings->value(controlEntryLastSelectedGUID).toString());
                data.open(QIODevice::Text | QIODevice::ReadOnly);
                QString dataText = data.readAll();

                QRegularExpression re(currentDevice->getGUIDString());
                QString replacementText(currentDevice->getUniqueIDString());

                dataText.replace(re, replacementText);

                QFile newData(m_settings->value(controlEntryLastSelectedGUID).toString());

                if (newData.open(QFile::WriteOnly | QFile::Truncate))
                {
                    QTextStream out(&newData);
                    out << dataText;
                }

                newData.close();

            break;
        }
}

void MainWindow::checkEachTenMinutesBattery(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    QMapIterator<SDL_JoystickID, InputDevice*> deviceIter(*joysticks);

    while (deviceIter.hasNext())
    {
        deviceIter.next();
        InputDevice *tempDevice = deviceIter.value();

        showBatteryLevel(SDL_JOYSTICK_POWER_LOW, "Low", "20%", tempDevice);
        showBatteryLevel(SDL_JOYSTICK_POWER_EMPTY, "Empty", "5%", tempDevice);
    }
}
