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

//#include <QDebug>
#include <QHash>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

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

#ifdef USE_SDL_2
#include "gamecontrollermappingdialog.h"

#if defined(WITH_X11) || defined(Q_OS_WIN)
#include "autoprofileinfo.h"
#endif

#endif

#ifdef Q_OS_WIN
#include "winextras.h"
#endif

#ifdef Q_OS_UNIX
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif
#endif

MainWindow::MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                       CommandLineUtility *cmdutility, AntiMicroSettings *settings,
                       bool graphical, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    this->translator = 0;
    this->appTranslator = 0;
    this->cmdutility = cmdutility;
    this->graphical = graphical;
    this->settings = settings;

    ui->actionStick_Pad_Assign->setVisible(false);

#ifndef USE_SDL_2
    ui->actionGameController_Mapping->setVisible(false);
#endif

#ifdef Q_OS_UNIX
    #if defined(USE_SDL_2) && defined(WITH_X11)
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    this->appWatcher = new AutoProfileWatcher(settings, this);
    checkAutoProfileWatcherTimer();
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    else
    {
        this->appWatcher = 0;
    }
        #endif
    #endif
#elif defined(Q_OS_WIN)
    this->appWatcher = new AutoProfileWatcher(settings, this);
    checkAutoProfileWatcherTimer();
#else
    this->appWatcher = 0;
#endif

    signalDisconnect = false;
    showTrayIcon = !cmdutility->isTrayHidden() && graphical &&
                   !cmdutility->shouldListControllers() && !cmdutility->shouldMapController();

    this->joysticks = joysticks;

    if (showTrayIcon)
    {
        trayIconMenu = new QMenu(this);
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setContextMenu(trayIconMenu);

        connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(refreshTrayIconMenu()));
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(trayIconClickAction(QSystemTrayIcon::ActivationReason)));
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
        aboutDialog = 0;
    }

    connect(ui->menuQuit, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->actionKeyValue, SIGNAL(triggered()), this, SLOT(openKeyCheckerDialog()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionProperties, SIGNAL(triggered()), this, SLOT(openJoystickStatusWindow()));
    connect(ui->actionGitHubPage, SIGNAL(triggered()), this, SLOT(openGitHubPage()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openMainSettingsDialog()));
    connect(ui->actionWiki, SIGNAL(triggered()), this, SLOT(openWikiPage()));

#ifdef USE_SDL_2
    connect(ui->actionGameController_Mapping, SIGNAL(triggered()), this, SLOT(openGameControllerMappingWindow()));

    #if defined(Q_OS_UNIX) && defined(WITH_X11)
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    connect(appWatcher, SIGNAL(foundApplicableProfile(AutoProfileInfo*)), this, SLOT(autoprofileLoad(AutoProfileInfo*)));
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #elif defined(Q_OS_WIN)
        connect(appWatcher, SIGNAL(foundApplicableProfile(AutoProfileInfo*)), this, SLOT(autoprofileLoad(AutoProfileInfo*)));
    #endif

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
            connect(ui->uacPushButton, SIGNAL(clicked()), this, SLOT(restartAsElevated()));
        }
        else
        {
            ui->uacPushButton->setVisible(false);
        }
    }

#else
    ui->uacPushButton->setVisible(false);
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::alterConfigFromSettings()
{
    if (cmdutility->shouldListControllers())
    {
        this->graphical = graphical = false;
    }
    else if (cmdutility->hasProfile())
    {
        if (cmdutility->hasControllerNumber())
        {
            loadConfigFile(cmdutility->getProfileLocation(),
                           cmdutility->getControllerNumber());
        }
        else if (cmdutility->hasControllerID())
        {
            loadConfigFile(cmdutility->getProfileLocation(),
                           cmdutility->hasControllerID());
        }
        else
        {
            loadConfigFile(cmdutility->getProfileLocation());
        }
    }

    QList<ControllerOptionsInfo> *tempList = cmdutility->getControllerOptionsList();
    //unsigned int optionListSize = tempList->size();

    QListIterator<ControllerOptionsInfo> optionIter(*tempList);
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

#ifdef USE_SDL_2
void MainWindow::controllerMapOpening()
{
    if (cmdutility->shouldMapController())
    {
        this->graphical = graphical = false;

        QList<ControllerOptionsInfo> *tempList = cmdutility->getControllerOptionsList();
        ControllerOptionsInfo temp = tempList->at(0);

        if (temp.hasControllerNumber())
        {
            unsigned int joypadIndex = cmdutility->getControllerNumber();
            selectControllerJoyTab(joypadIndex);
            openGameControllerMappingWindow(true);
        }
        else if (temp.hasControllerID())
        {
            QString joypadGUID = cmdutility->getControllerID();
            selectControllerJoyTab(joypadGUID);
            openGameControllerMappingWindow(true);
        }
        else
        {
            Logger::LogInfo(tr("Could not find a proper controller identifier. "
                               "Exiting."));
            qApp->quit();
        }
    }
}
#endif

void MainWindow::fillButtons()
{
    fillButtons(joysticks);
}

void MainWindow::makeJoystickTabs()
{
    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

#ifdef USE_SDL_2
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
#else
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
#endif

    while (iter.hasNext())
    {
        iter.next();

        InputDevice *joystick = iter.value();
        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(tr("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
    }

    if (joysticks > 0)
    {
        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::fillButtons(InputDevice *joystick)
{
    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));
    tabwidget->refreshButtons();
}

void MainWindow::fillButtons(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

#ifdef USE_SDL_2
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
#else
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
#endif

    while (iter.hasNext())
    {
        iter.next();

        InputDevice *joystick = iter.value();

        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(tr("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
        tabwidget->refreshButtons();
        connect(tabwidget, SIGNAL(namesDisplayChanged(bool)), this, SLOT(propogateNameDisplayStatus(bool)));
#ifdef USE_SDL_2
        connect(tabwidget, SIGNAL(mappingUpdated(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString,InputDevice*)));
#endif

        if (showTrayIcon)
        {
            connect(tabwidget, SIGNAL(joystickConfigChanged(int)), this, SLOT(populateTrayIcon()));
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
    ui->stackedWidget->setCurrentIndex(0);
    ui->actionUpdate_Joysticks->setEnabled(false);
    ui->actionHide->setEnabled(false);
    ui->actionQuit->setEnabled(false);
    removeJoyTabs();

    emit joystickRefreshRequested();
}

void MainWindow::populateTrayIcon()
{
    disconnect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(singleTrayProfileMenuShow()));

    trayIconMenu->clear();
    profileActions.clear();
    unsigned int joystickCount = joysticks->size();

    if (joystickCount > 0)
    {
        QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
        bool useSingleList = settings->value("TrayProfileList", false).toBool();
        if (!useSingleList && joystickCount == 1)
        {
            useSingleList = true;
        }

        int i = 0;
        while (iter.hasNext())
        {
            iter.next();
            InputDevice *current = iter.value();

            QString joytabName = current->getSDLName();
            joytabName.append(" ").append(tr("(%1)").arg(current->getName()));
            QMenu *joysticksubMenu = 0;

            if (!useSingleList)
            {
                joysticksubMenu = trayIconMenu->addMenu(joytabName);
            }

            JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
            if (widget)
            {
                QHash<int, QString> *configs = widget->recentConfigs();
                QHashIterator<int, QString> configIter(*configs);
                QList<QAction*> tempProfileList;
                while (configIter.hasNext())
                {
                    configIter.next();
                    QAction *newaction = 0;
                    if (joysticksubMenu)
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
                    connect(newaction, SIGNAL(triggered(bool)), this, SLOT(profileTrayActionTriggered(bool)));

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
                configs = 0;

                QAction *newaction = 0;
                if (joysticksubMenu)
                {
                    newaction = new QAction(tr("Open File"), joysticksubMenu);
                }
                else
                {
                    newaction = new QAction(tr("Open File"), trayIconMenu);
                }

                newaction->setIcon(QIcon::fromTheme("document-open"));
                connect(newaction, SIGNAL(triggered()), widget, SLOT(openConfigFileDialog()));

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
                    connect(joysticksubMenu, SIGNAL(aboutToShow()), this, SLOT(joystickTrayShow()));
                }

                i++;
            }
        }

        if (useSingleList)
        {
            connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(singleTrayProfileMenuShow()));
        }

        trayIconMenu->addSeparator();
    }

    hideAction = new QAction(tr("&Hide"), trayIconMenu);
    hideAction->setIcon(QIcon::fromTheme("view-restore"));
    connect(hideAction, SIGNAL(triggered()), this, SLOT(hideWindow()));

    restoreAction = new QAction(tr("&Restore"), trayIconMenu);
    restoreAction->setIcon(QIcon::fromTheme("view-fullscreen"));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    closeAction = new QAction(tr("&Quit"), trayIconMenu);
    closeAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(quitProgram()));

    updateJoy = new QAction(tr("&Update Joysticks"), trayIconMenu);
    updateJoy->setIcon(QIcon::fromTheme("view-refresh"));
    connect(updateJoy, SIGNAL(triggered()), this, SLOT(startJoystickRefresh()));

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
    bool discard = true;
    for (int i=0; i < ui->tabWidget->count() && discard; i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        discard = tab->discardUnsavedProfileChanges();
    }

    if (discard)
    {
        qApp->quit();
    }
}

void MainWindow::refreshTrayIconMenu()
{
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

void MainWindow::mainMenuChange()
{
    QMenu *tempMenu = static_cast<QMenu*>(sender());

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

#ifndef USE_SDL_2
    if (tempMenu == ui->menuOptions)
    {
        ui->actionGameController_Mapping->setVisible(false);
    }
#endif
}

void MainWindow::saveAppConfig()
{
    if (joysticks->size() > 0)
    {
        JoyTabWidget *temptabwidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(0));
        settings->setValue("DisplayNames",
            temptabwidget->isDisplayingNames() ? "1" : "0");

        settings->beginGroup("Controllers");
        QStringList tempIdentifierHolder;

        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            bool prepareSave = true;

            JoyTabWidget *tabwidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
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

        settings->endGroup();
    }

    settings->setValue("WindowSize", size());
    settings->setValue("WindowPosition", pos());
}

void MainWindow::loadAppConfig(bool forceRefresh)
{
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tabwidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        tabwidget->loadSettings(forceRefresh);
    }
}

void MainWindow::disableFlashActions()
{
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

        JoyTabWidget *tabWidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        ui->tabWidget->disableFlashes(tabWidget->getJoystick());
    }
}

void MainWindow::enableFlashActions()
{
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

        JoyTabWidget *tabWidget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        ui->tabWidget->enableFlashes(tabWidget->getJoystick());
    }
}

// Intermediate slot used in Design mode
void MainWindow::hideWindow()
{
    disableFlashActions();
    signalDisconnect = true;
    hide();
}

void MainWindow::joystickTrayShow()
{
    QMenu *tempmenu = static_cast<QMenu*>(sender());
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
            JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));

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
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *e = static_cast<QWindowStateChangeEvent*>(event);
        if (e->oldState() != Qt::WindowMinimized && isMinimized())
        {
            bool minimizeToTaskbar = settings->value("MinimizeToTaskbar", false).toBool();
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
    aboutDialog->show();
}

void MainWindow::loadConfigFile(QString fileLocation, int joystickIndex)
{
    if (joystickIndex > 0 && joysticks->contains(joystickIndex-1))
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joystickIndex-1));
        if (widget)
        {
            widget->loadConfigFile(fileLocation);
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
            if (widget)
            {
                widget->loadConfigFile(fileLocation);
            }
        }
    }
}

void MainWindow::loadConfigFile(QString fileLocation, QString controllerID)
{
    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab)
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
    int oldtabcount = ui->tabWidget->count();

    for (int i = oldtabcount-1; i >= 0; i--)
    {
        QWidget *tab = ui->tabWidget->widget(i);
        delete tab;
        tab = 0;
    }

    ui->tabWidget->clear();
}

void MainWindow::handleInstanceDisconnect()
{
    settings->sync();
    loadAppConfig(true);
}

void MainWindow::openJoystickStatusWindow()
{
    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(index));
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick)
        {
            JoystickStatusWindow *dialog = new JoystickStatusWindow(joystick, this);
            dialog->show();
        }
    }
}

void MainWindow::openKeyCheckerDialog()
{
    QKeyDisplayDialog *dialog = new QKeyDisplayDialog(this);
    dialog->show();
}

void MainWindow::openGitHubPage()
{
    QDesktopServices::openUrl(QUrl(PadderCommon::githubProjectPage));
}

void MainWindow::openWikiPage()
{
    QDesktopServices::openUrl(QUrl(PadderCommon::wikiPage));
}

void MainWindow::unloadCurrentConfig(int joystickIndex)
{
    if (joystickIndex > 0 && joysticks->contains(joystickIndex-1))
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*> (ui->tabWidget->widget(joystickIndex-1));
        if (widget)
        {
            widget->unloadConfig();
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = static_cast<JoyTabWidget*> (ui->tabWidget->widget(i));
            if (widget)
            {
                widget->unloadConfig();
            }
        }
    }
}

void MainWindow::unloadCurrentConfig(QString controllerID)
{
    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab)
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

void MainWindow::propogateNameDisplayStatus(bool displayNames)
{
    JoyTabWidget *tabwidget = static_cast<JoyTabWidget*>(sender());
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab && tab != tabwidget)
        {
            if (tab->isDisplayingNames() != displayNames)
            {
                tab->changeNameDisplay(displayNames);
            }
        }
    }
}

void MainWindow::changeStartSetNumber(unsigned int startSetNumber, QString controllerID)
{
    if (!controllerID.isEmpty())
    {
        QListIterator<JoyTabWidget*> iter(ui->tabWidget->findChildren<JoyTabWidget*>());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = iter.next();
            if (tab)
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

void MainWindow::changeStartSetNumber(unsigned int startSetNumber, unsigned int joystickIndex)
{
    if (joystickIndex > 0 && joysticks->contains(joystickIndex-1))
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joystickIndex-1));
        if (widget)
        {
            widget->changeCurrentSet(startSetNumber);
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
            if (widget)
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
    QList<InputDevice*> *devices = new QList<InputDevice*>(joysticks->values());
    MainSettingsDialog *dialog = new MainSettingsDialog(settings, devices, this);
    connect(dialog, SIGNAL(changeLanguage(QString)), this, SLOT(changeLanguage(QString)));

    if (appWatcher)
    {
#if defined(USE_SDL_2) && defined(Q_OS_UNIX) && defined(WITH_X11)
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    connect(dialog, SIGNAL(accepted()), appWatcher, SLOT(syncProfileAssignment()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkAutoProfileWatcherTimer()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(checkAutoProfileWatcherTimer()));
    appWatcher->stopTimer();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif

#elif defined(USE_SDL_2) && defined(Q_OS_WIN)
    connect(dialog, SIGNAL(accepted()), appWatcher, SLOT(syncProfileAssignment()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkAutoProfileWatcherTimer()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(checkAutoProfileWatcherTimer()));
    appWatcher->stopTimer();
#endif
    }

    connect(dialog, SIGNAL(accepted()), this, SLOT(populateTrayIcon()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkHideEmptyOption()));

#ifdef Q_OS_WIN
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkKeyRepeatOptions()));

#endif

    dialog->show();
}

/**
 * @brief Change language used by the application.
 * @param Language code
 */
void MainWindow::changeLanguage(QString language)
{
    if (translator && appTranslator)
    {
        PadderCommon::reloadTranslations(translator, appTranslator, language);
    }
}

/**
 * @brief Check if the program should really quit or if it should
 *     be minimized.
 * @param QCloseEvent
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    bool closeToTray = settings->value("CloseToTray", false).toBool();
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
    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(index));
        Joystick *joystick = static_cast<Joystick*>(joyTab->getJoystick());

        AdvanceStickAssignmentDialog *dialog = new AdvanceStickAssignmentDialog(joystick, this);
        connect(dialog, SIGNAL(finished(int)), joyTab, SLOT(fillButtons()));
        dialog->show();
    }
}

/**
 * @brief Display a version of the tray menu that shows all recent profiles for
 *    all controllers in one list.
 */
void MainWindow::singleTrayProfileMenuShow()
{
    if (!profileActions.isEmpty())
    {
        QMapIterator<int, QList<QAction*> > mapIter(profileActions);
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
                    JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));

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


void MainWindow::profileTrayActionTriggered(bool checked)
{
    // Obtaining the selected config
    QAction *action = static_cast<QAction*>(sender());
    QHash<QString, QVariant> tempmap = action->data().toHash();
    QHashIterator<QString, QVariant> iter(tempmap);
    while (iter.hasNext())
    {
        iter.next();

        // Fetching indicies and tab associated with the current joypad
        int joyindex = iter.key().toInt();
        int configindex = iter.value().toInt();
        JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));

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
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab)
        {
            tab->checkHideEmptyOption();
        }
    }
}


#ifdef Q_OS_WIN
void MainWindow::checkKeyRepeatOptions()
{
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        tab->deviceKeyRepeatSettings();
    }
}

/**
 * @brief Check if user really wants to restart the program with elevated
 *     privileges. If yes, attempt to restart the program.
 */
void MainWindow::restartAsElevated()
{
    QMessageBox msg;
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setWindowTitle(tr("Run as Administrator?"));
    msg.setText(tr("Are you sure that you want to run this program as Adminstrator?"
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
            msg.setWindowTitle(tr("Failed to elevate program"));
            msg.setText(tr("Failed to restart this program as the Administrator"));
            msg.exec();
        }
    }
}

#endif

#ifdef USE_SDL_2
void MainWindow::openGameControllerMappingWindow(bool openAsMain)
{
    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(index));
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick)
        {
            GameControllerMappingDialog *dialog = new GameControllerMappingDialog(joystick, settings, this);

            if (openAsMain)
            {
                dialog->setParent(0);
                dialog->setWindowFlags(Qt::Window);
                connect(dialog, SIGNAL(finished(int)), qApp, SLOT(quit()));
            }
            else
            {
                connect(dialog, SIGNAL(mappingUpdate(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString, InputDevice*)));
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
    emit mappingUpdated(mapping, device);
}

void MainWindow::testMappingUpdateNow(int index, InputDevice *device)
{
    QWidget *tab = ui->tabWidget->widget(index);
    if (tab)
    {
        ui->tabWidget->removeTab(index);
        delete tab;
        tab = 0;
    }

    JoyTabWidget *tabwidget = new JoyTabWidget(device, settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
    ui->tabWidget->insertTab(index, tabwidget, joytabName);
    tabwidget->refreshButtons();
    ui->tabWidget->setCurrentIndex(index);

    connect(tabwidget, SIGNAL(namesDisplayChanged(bool)), this, SLOT(propogateNameDisplayStatus(bool)));
    connect(tabwidget, SIGNAL(mappingUpdated(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString,InputDevice*)));
    if (showTrayIcon)
    {
        connect(tabwidget, SIGNAL(joystickConfigChanged(int)), this, SLOT(populateTrayIcon()));
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
    }
}

void MainWindow::removeJoyTab(SDL_JoystickID deviceID)
{
    bool found = false;
    for (int i=0; i < ui->tabWidget->count() && !found; i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab && deviceID == tab->getJoystick()->getSDLJoystickID())
        {
            // Save most recent profile list to settings before removing tab.
            tab->saveDeviceSettings();

            // Remove flash event connections between buttons and
            // the tab before deleting tab.
            ui->tabWidget->disableFlashes(tab->getJoystick());
            ui->tabWidget->removeTab(i);
            QMetaObject::invokeMethod(tab->getJoystick(), "finalRemoval");
            delete tab;
            tab = 0;
            found = true;
        }
    }

    // Refresh tab text to reflect new index values.
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab)
        {
            InputDevice *device = tab->getJoystick();
            QString joytabName = device->getSDLName();
            joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
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
    JoyTabWidget *tabwidget = new JoyTabWidget(device, settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
    ui->tabWidget->addTab(tabwidget, joytabName);
    tabwidget->loadDeviceSettings();
    tabwidget->refreshButtons();

    // Refresh tab text to reflect new index values.
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab)
        {
            InputDevice *device = tab->getJoystick();
            QString joytabName = device->getSDLName();
            joytabName.append(" ").append(tr("(%1)").arg(device->getName()));
            ui->tabWidget->setTabText(i, joytabName);
        }
    }

    connect(tabwidget, SIGNAL(namesDisplayChanged(bool)), this, SLOT(propogateNameDisplayStatus(bool)));
    connect(tabwidget, SIGNAL(mappingUpdated(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString,InputDevice*)));
    if (showTrayIcon)
    {
        connect(tabwidget, SIGNAL(joystickConfigChanged(int)), this, SLOT(populateTrayIcon()));
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
    }

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::autoprofileLoad(AutoProfileInfo *info)
{
  if( info != NULL ) {
    Logger::LogDebug(QObject::tr("Auto-switching to profile \"%1\".").
		     arg(info->getProfileLocation()));
  } else {
    Logger::LogError(QObject::tr("Auto-switching to NULL profile!"));    
  }
  
#if defined(USE_SDL_2) && (defined(WITH_X11) || defined(Q_OS_WIN))
    #if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (widget)
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
                    }
                }

                delete customs;
                customs = 0;

                // Check if profile has already been switched for a particular
                // controller.
                if (!found)
                {
                    QString tempguid = widget->getJoystick()->getGUIDString();
                    if (appWatcher->isGUIDLocked(tempguid))
                    {
                        found = true;
                    }
                }

                if (!found)
                {
                    // If the profile location is empty, assume
                    // that an empty profile should get loaded.
                    if (info->getProfileLocation().isEmpty())
                    {
                        widget->setCurrentConfig(0);
                    }
                    else
                    {
                        widget->loadConfigFile(info->getProfileLocation());
                    }
                }
            }
            else if (info->getGUID() == widget->getJoystick()->getStringIdentifier())
            {
                if (info->getProfileLocation().isEmpty())
                {
                    widget->setCurrentConfig(0);
                }
                else
                {
                    widget->loadConfigFile(info->getProfileLocation());
                }
            }
        }
    }
    #if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif
}

void MainWindow::checkAutoProfileWatcherTimer()
{
#if defined(USE_SDL_2) && (defined(WITH_X11) || defined(Q_OS_WIN))
    #if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    QString autoProfileActive = settings->value("AutoProfiles/AutoProfilesActive", "0").toString();
    if (autoProfileActive == "1")
    {
        appWatcher->startTimer();
    }
    else
    {
        appWatcher->stopTimer();
    }
    #if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif
}

/**
 * @brief TODO: Check if method is save to remove.
 */
void MainWindow::updateMenuOptions()
{
    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(index));
        InputDevice *joystick = joyTab->getJoystick();

        if (qobject_cast<GameController*>(joystick) != 0)
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
void MainWindow::selectControllerJoyTab(unsigned int index)
{
    if (index > 0 && joysticks->contains(index-1))
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*> (ui->tabWidget->widget(index-1));
        if (widget)
        {
            ui->tabWidget->setCurrentIndex(index-1);
        }
    }
}

/**
 * @brief Select appropriate tab that has a device with the specified GUID.
 * @param GUID of joystick device.
 */
void MainWindow::selectControllerJoyTab(QString GUID)
{
    if (!GUID.isEmpty())
    {
        InputDevice *device = 0;
        QMapIterator<SDL_JoystickID, InputDevice*> deviceIter(*joysticks);
        while (deviceIter.hasNext())
        {
            deviceIter.next();
            InputDevice *tempDevice = deviceIter.value();
            if (tempDevice && GUID == tempDevice->getStringIdentifier())
            {
                device = tempDevice;
                deviceIter.toBack();
            }
        }

        if (device)
        {
            ui->tabWidget->setCurrentIndex(device->getJoyNumber());
        }
    }
}

#endif

void MainWindow::changeWindowStatus()
{
    // Check flags to see if user requested for the main window and the tray icon
    // to not be displayed.
    if (graphical)
    {
        bool launchInTraySetting = settings->runtimeValue("LaunchInTray", false).toBool();
        if (!cmdutility->isHiddenRequested() &&
            (!launchInTraySetting || !QSystemTrayIcon::isSystemTrayAvailable()))
        {
            show();
        }
        else if (cmdutility->isHiddenRequested() && cmdutility->isTrayHidden())
        {
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();

            setEnabled(false); // Should already be disabled. Do it again just to be sure.
        }
        else if (cmdutility->isHiddenRequested() || launchInTraySetting)
        {
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();
        }
    }
}

bool MainWindow::getGraphicalStatus()
{
    return graphical;
}

void MainWindow::setTranslator(QTranslator *translator)
{
    this->translator = translator;
}

QTranslator* MainWindow::getTranslator()
{
    return translator;
}

void MainWindow::setAppTranslator(QTranslator *translator)
{
    this->appTranslator = translator;
}

QTranslator* MainWindow::getAppTranslator()
{
    return appTranslator;
}

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
}

void MainWindow::refreshTabHelperThreads()
{
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *widget = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (widget)
        {
            widget->refreshHelperThread();
        }
    }
}
