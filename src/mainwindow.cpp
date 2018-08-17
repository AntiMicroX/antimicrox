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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "messagehandler.h"
#include "inputdevice.h"
#include "aboutdialog.h"
#include "commandlineutility.h"
#include "antimicrosettings.h"
#include "autoprofilewatcher.h"
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

#if defined(WITH_X11) || defined(Q_OS_WIN)
#include "autoprofileinfo.h"
#endif

#ifdef Q_OS_WIN
    #include "winextras.h"
#endif

#ifdef Q_OS_UNIX
#include <QApplication>
#endif

#include <QHideEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QLocalServer>
#include <QTranslator>
#include <QAction>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include <QHashIterator>
#include <QMapIterator>
#include <QLocalSocket>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QLibraryInfo>

#ifdef Q_OS_WIN
#include <QSysInfo>
#endif

MainWindow::MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                       CommandLineUtility *cmdutility, AntiMicroSettings *settings,
                       bool graphical, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    ui->stackedWidget->setCurrentIndex(0);

    m_translator = nullptr;
    m_appTranslator = nullptr;
    m_cmdutility = cmdutility;
    m_graphical = graphical;
    m_settings = settings;

    ui->actionStick_Pad_Assign->setVisible(false);

#ifdef Q_OS_UNIX
    #if defined(USE_SDL_2) && defined(WITH_X11)
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
#elif defined(Q_OS_WIN)
    this->appWatcher = new AutoProfileWatcher(settings, this);
    checkAutoProfileWatcherTimer();
#else
    this->appWatcher = nullptr;
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

    #if defined(Q_OS_UNIX) && defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    connect(appWatcher, &AutoProfileWatcher::foundApplicableProfile, this, &MainWindow::autoprofileLoad);
    }
    #elif defined(Q_OS_WIN)
        connect(appWatcher, &AutoProfileWatcher::foundApplicableProfile, this, &MainWindow::autoprofileLoad);
    #endif

#ifdef Q_OS_WIN
    if (graphical)
    {
        if (!WinExtras::IsRunningAsAdmin())
        {
            if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
            {
                QIcon uacIcon = QApplication::style()->standardIcon(QStyle::SP_VistaShield);
                ui->uacPushButton->setIcon(uacIcon);
            }
            connect(ui->uacPushButton, &QPushButton::clicked, this, &MainWindow::restartAsElevated);
        }
        else
        {
            ui->uacPushButton->setVisible(false);
        }
    }

#elif defined(Q_OS_UNIX)
    ui->uacPushButton->setVisible(false);
#endif

}

MainWindow::~MainWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It was antimicro --map controllerNumber";
            qDebug() << "controllerNumber: " << joypadIndex;
            #endif

            selectControllerJoyTab(joypadIndex);
            openGameControllerMappingWindow(true);
        }
        else if (temp.hasControllerID())
        {


            QString joypadGUID = m_cmdutility->getControllerID();

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It was antimicro --map controllerID";
            qDebug() << "controllerID: " << joypadGUID;
            #endif

            selectControllerJoyTab(joypadGUID);
            openGameControllerMappingWindow(true);
        }
        else
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Could not find a proper controller identifier. Exiting";
            #endif

            Logger::LogInfo(trUtf8("Could not find a proper controller identifier. "
                               "Exiting."));
            qApp->quit();
        }
    }
}


void MainWindow::fillButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    fillButtons(m_joysticks);
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
        joytabName.append(" ").append(trUtf8("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
    }

    if (m_joysticks != nullptr)
    {
        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::fillButtons(InputDevice *joystick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));  // static_cast
    tabwidget->refreshButtons();
}

void MainWindow::fillButtons(QMap<SDL_JoystickID, InputDevice *> *joysticks)
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
        joytabName.append(" ").append(trUtf8("(%1)").arg(joystick->getName()));
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
        trayIcon->hide();
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
    int joystickCount = m_joysticks->size();

    if (joystickCount > 0)
    {
        QMapIterator<SDL_JoystickID, InputDevice*> iter(*m_joysticks);
        bool useSingleList = m_settings->value("TrayProfileList", false).toBool();
        if (!useSingleList && (joystickCount == 1))
        {
            useSingleList = true;
        }

        int i = 0;
        while (iter.hasNext())
        {
            iter.next();
            InputDevice *current = iter.value();

            QString joytabName = current->getSDLName();
            joytabName.append(" ").append(trUtf8("(%1)").arg(current->getName()));
            QMenu *joysticksubMenu = nullptr;

            if (!useSingleList)
            {
                joysticksubMenu = trayIconMenu->addMenu(joytabName);
            }

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

                    if (configIter.key() == widget->getCurrentConfigIndex())
                    {
                        newaction->setChecked(true);
                    }

                    QHash<QString, QVariant> tempmap;
                    tempmap.insert(QString::number(i), QVariant (configIter.key()));
                    QVariant tempvar (tempmap);
                    newaction->setData(tempvar);
                    connect(newaction, &QAction::triggered, this, [this, newaction](bool checked) {
                        profileTrayActionTriggered(newaction, checked);
                    });

                    if (useSingleList)
                    {
                        tempProfileList.append(newaction);
                    }
                    else
                    {
                        joysticksubMenu->addAction(newaction);
                    }
                }

                delete configs;
                configs = nullptr;

                QAction *newaction = nullptr;
                if (joysticksubMenu != nullptr)
                {
                    newaction = new QAction(trUtf8("Open File"), joysticksubMenu);
                }
                else
                {
                    newaction = new QAction(trUtf8("Open File"), trayIconMenu);
                }

                newaction->setIcon(QIcon::fromTheme("document-open"));
                connect(newaction, &QAction::triggered, widget, &JoyTabWidget::openConfigFileDialog);

                if (useSingleList)
                {
                    QAction *titleAction = new QAction(joytabName, trayIconMenu);
                    titleAction->setCheckable(false);

                    QFont actionFont = titleAction->font();
                    actionFont.setBold(true);
                    titleAction->setFont(actionFont);

                    trayIconMenu->addAction(titleAction);
                    trayIconMenu->addActions(tempProfileList);
                    trayIconMenu->addAction(newaction);

                    profileActions.insert(i, tempProfileList);

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

    hideAction = new QAction(trUtf8("&Hide"), trayIconMenu);
    hideAction->setIcon(QIcon::fromTheme("view-restore"));
    connect(hideAction, &QAction::triggered, this, &MainWindow::hideWindow);

    restoreAction = new QAction(trUtf8("&Restore"), trayIconMenu);
    restoreAction->setIcon(QIcon::fromTheme("view-fullscreen"));
    connect(restoreAction, &QAction::triggered, this, &MainWindow::show);

    closeAction = new QAction(trUtf8("&Quit"), trayIconMenu);
    closeAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(closeAction, &QAction::triggered, this, &MainWindow::quitProgram);

    updateJoy = new QAction(trUtf8("&Update Joysticks"), trayIconMenu);
    updateJoy->setIcon(QIcon::fromTheme("view-refresh"));
    connect(updateJoy, &QAction::triggered, this, &MainWindow::startJoystickRefresh);

    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(updateJoy);
    trayIconMenu->addAction(closeAction);

    QIcon icon = QIcon::fromTheme("antimicro", QIcon(":/images/antimicro_trayicon.png"));
    trayIcon->setIcon(icon);
    trayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::quitProgram()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool discard = true;
    for (int i=0; i < ui->tabWidget->count() && discard; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        discard = tab->discardUnsavedProfileChanges();
    }

    if (discard)
    {
        qApp->quit();
    }
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
                    action->setIcon(QIcon::fromTheme("document-save-as"));
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

    bool propogate = true;

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

    if (propogate)
    {
        QMainWindow::showEvent(event);
    }
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

        QMessageBox::information(this, trUtf8("Calibration couldn't be opened"), trUtf8("You must connect at least one controller to open the window"));

    } else {

        QPointer<Calibration> calibration = new Calibration(m_joysticks);
        calibration.data()->show();

        if (calibration.isNull())
            calibration.clear();

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
#if defined(USE_SDL_2) && defined(Q_OS_UNIX) && defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    connect(dialog, &MainSettingsDialog::accepted, appWatcher, &AutoProfileWatcher::syncProfileAssignment);
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkAutoProfileWatcherTimer);
    connect(dialog, &MainSettingsDialog::rejected, this, &MainWindow::checkAutoProfileWatcherTimer);
    appWatcher->stopTimer();
    qDebug() << "Stopping appWatcher in openMainSettingsDialog";
    }

#elif defined(USE_SDL_2) && defined(Q_OS_WIN)
    connect(dialog, &MainSettingsDialog::accepted, appWatcher, &AutoProfileWatcher::syncProfileAssignment);
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkAutoProfileWatcherTimer);
    connect(dialog, &MainSettingsDialog::rejected, this, &MainWindow::checkAutoProfileWatcherTimer);
    appWatcher->stopTimer();
#endif
    }

    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::populateTrayIcon);
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkHideEmptyOption);

#ifdef Q_OS_WIN
    connect(dialog, &MainSettingsDialog::accepted, this, &MainWindow::checkKeyRepeatOptions);

#endif

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
                            action->setIcon(QIcon::fromTheme("document-save-as"));
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


#ifdef Q_OS_WIN
void MainWindow::checkKeyRepeatOptions()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        tab->deviceKeyRepeatSettings();
    }
}

/**
 * @brief Check if user really wants to restart the program with elevated
 *     privileges. If yes, attempt to restart the program.
 */
void MainWindow::restartAsElevated()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msg;
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setWindowTitle(trUtf8("Run as Administrator?"));
    msg.setText(trUtf8("Are you sure that you want to run this program as Adminstrator?"
                   "\n\n"
                   "Some games run as Administrator which will cause events generated by antimicro "
                   "to not be used by those games unless antimicro is also run "
                   "as the Adminstrator. "
                   "This is due to permission problems caused by User Account "
                   "Control (UAC) options in Windows Vista and later."));

    if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
    {
        QIcon uacIcon = QApplication::style()->standardIcon(QStyle::SP_VistaShield);
        msg.button(QMessageBox::Yes)->setIcon(uacIcon);
    }

    int result = msg.exec();
    if (result == QMessageBox::Yes)
    {
        bool result = WinExtras::elevateAntiMicro();
        if (result)
        {
            qApp->quit();
        }
        else
        {
            msg.setStandardButtons(QMessageBox::Close);
            msg.setWindowTitle(trUtf8("Failed to elevate program"));
            msg.setText(trUtf8("Failed to restart this program as the Administrator"));
            msg.exec();
        }
    }
}

#endif

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
        Logger::LogInfo(trUtf8("Could not find controller. Exiting."));
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
    joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
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
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
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
            joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
            ui->tabWidget->setTabText(i, joytabName);
        }
    }

    if (showTrayIcon)
    {
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
    }

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
    joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
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
            joytabName_in_loop.append(" ").append(trUtf8("(%1)").arg(device_in_loop->getName()));
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
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
    }

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::autoprofileLoad(AutoProfileInfo *info)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

  if( info != nullptr ) {
    Logger::LogDebug(QObject::trUtf8("Auto-switching to profile \"%1\".").
		     arg(info->getProfileLocation()));
  } else {
    Logger::LogError(QObject::trUtf8("Auto-switching to nullptr profile!"));
  }
  
#if defined(USE_SDL_2) && (defined(WITH_X11) || defined(Q_OS_WIN))
    #if defined(Q_OS_UNIX)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast
        if (widget != nullptr)
        {
            if (info->getGUID() == "all")
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
                    if (tempinfo->getGUID() == widget->getJoystick()->getGUIDString() &&
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
                    QString tempguid = widget->getJoystick()->getGUIDString();
                    if (appWatcher->isGUIDLocked(tempguid))
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
            else if (info->getGUID() == widget->getJoystick()->getStringIdentifier())
            {
                qDebug() << "GUID of AutoProfileInfo: " << info->getGUID() << " == string identifier of AutoProfileInfo: " << widget->getJoystick()->getStringIdentifier();
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
    #if defined(Q_OS_UNIX)
    }
    #endif
#endif
}

void MainWindow::checkAutoProfileWatcherTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

#if defined(USE_SDL_2) && (defined(WITH_X11) || defined(Q_OS_WIN))
    #if defined(Q_OS_UNIX)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
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
    #if defined(Q_OS_UNIX)
    }
    #endif
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
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "JoyTabWidget was not a null pointer in selectControllerJoyTab of index";
            #endif
            ui->tabWidget->setCurrentIndex(index - 1);
        }
        else
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "JoyTabWidget was a NULL POINTER in selectControllerJoyTab of index";
            #endif
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
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "InputDevice was not a null pointer in selectControllerJoyTab of GUID";
            #endif

            ui->tabWidget->setCurrentIndex(device->getJoyNumber());
        }
        else
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "InputDevice was a NULL POINTER in selectControllerJoyTab of GUID";
            #endif
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
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i));  // static_cast
        if (widget != nullptr)
        {
            widget->refreshHelperThread();
        }
    }
}

QMap<int, QList<QAction*> > const& MainWindow::getProfileActions() {

    return profileActions;
}
