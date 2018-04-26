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

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    ui->stackedWidget->setCurrentIndex(0);

    this->translator = nullptr;
    this->appTranslator = nullptr;
    this->cmdutility = cmdutility;
    this->graphical = graphical;
    this->settings = settings;

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
        aboutDialog = nullptr;
    }

    connect(ui->menuQuit, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->actionKeyValue, SIGNAL(triggered()), this, SLOT(openKeyCheckerDialog()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionProperties, SIGNAL(triggered()), this, SLOT(openJoystickStatusWindow()));
    connect(ui->actionGitHubPage, SIGNAL(triggered()), this, SLOT(openGitHubPage()));
    connect(ui->actionIssues, SIGNAL(triggered()), this, SLOT(openIssuesPage()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openMainSettingsDialog()));
    connect(ui->actionWiki, SIGNAL(triggered()), this, SLOT(openWikiPage()));
    connect(ui->actionGameController_Mapping, SIGNAL(triggered()), this, SLOT(openGameControllerMappingWindow()));

    #if defined(Q_OS_UNIX) && defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    connect(appWatcher, SIGNAL(foundApplicableProfile(AutoProfileInfo*)), this, SLOT(autoprofileLoad(AutoProfileInfo*)));
    }
    #elif defined(Q_OS_WIN)
        connect(appWatcher, SIGNAL(foundApplicableProfile(AutoProfileInfo*)), this, SLOT(autoprofileLoad(AutoProfileInfo*)));
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

#elif defined(Q_OS_UNIX)
    ui->uacPushButton->setVisible(false);
#endif

}

MainWindow::~MainWindow()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    delete ui;
}

void MainWindow::alterConfigFromSettings()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (cmdutility->shouldListControllers())
    {
        graphical = false;
        this->graphical = graphical;
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

    const QList<ControllerOptionsInfo> tempList = cmdutility->getControllerOptionsList();
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (cmdutility->shouldMapController())
    {
        graphical = false;
        this->graphical = graphical;

        const QList<ControllerOptionsInfo> tempList = cmdutility->getControllerOptionsList();
        ControllerOptionsInfo temp = tempList.at(0);

        if (temp.hasControllerNumber())
        {
            int joypadIndex = cmdutility->getControllerNumber();
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
            Logger::LogInfo(trUtf8("Could not find a proper controller identifier. "
                               "Exiting."));
            qApp->quit();
        }
    }
}


void MainWindow::fillButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    fillButtons(joysticks);
}

void MainWindow::makeJoystickTabs()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(trUtf8("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
    }

    if (joysticks != nullptr)
    {
        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::fillButtons(InputDevice *joystick)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(joyindex));  // static_cast
    tabwidget->refreshButtons();
}

void MainWindow::fillButtons(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(trUtf8("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
        tabwidget->refreshButtons();
        connect(tabwidget, SIGNAL(namesDisplayChanged(bool)), this, SLOT(propogateNameDisplayStatus(bool)));
        connect(tabwidget, SIGNAL(mappingUpdated(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString,InputDevice*)));

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->stackedWidget->setCurrentIndex(0);
    ui->actionUpdate_Joysticks->setEnabled(false);
    ui->actionHide->setEnabled(false);
    ui->actionQuit->setEnabled(false);
    removeJoyTabs();

    emit joystickRefreshRequested();
}

void MainWindow::populateTrayIcon()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(singleTrayProfileMenuShow()));

    trayIconMenu->clear();
    profileActions.clear();
    int joystickCount = joysticks->size();

    if (joystickCount > 0)
    {
        QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
        bool useSingleList = settings->value("TrayProfileList", false).toBool();
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

    hideAction = new QAction(trUtf8("&Hide"), trayIconMenu);
    hideAction->setIcon(QIcon::fromTheme("view-restore"));
    connect(hideAction, SIGNAL(triggered()), this, SLOT(hideWindow()));

    restoreAction = new QAction(trUtf8("&Restore"), trayIconMenu);
    restoreAction->setIcon(QIcon::fromTheme("view-fullscreen"));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    closeAction = new QAction(trUtf8("&Quit"), trayIconMenu);
    closeAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(quitProgram()));

    updateJoy = new QAction(trUtf8("&Update Joysticks"), trayIconMenu);
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QMenu *tempMenu = qobject_cast<QMenu*>(sender()); // static_cast

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (joysticks->size() > 0)
    {
        JoyTabWidget *temptabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(0)); // static_cast
        settings->setValue("DisplayNames",
            temptabwidget->isDisplayingNames() ? "1" : "0");

        settings->beginGroup("Controllers");
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

        settings->endGroup();
    }

    settings->setValue("WindowSize", size());
    settings->setValue("WindowPosition", pos());
}

void MainWindow::loadAppConfig(bool forceRefresh)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        tabwidget->loadSettings(forceRefresh);
    }
}

void MainWindow::disableFlashActions()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disableFlashActions();
    signalDisconnect = true;
    hide();
}

void MainWindow::joystickTrayShow()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QMenu *tempmenu = qobject_cast<QMenu*>(sender()); // static_cast
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    aboutDialog->show();
}

void MainWindow::loadConfigFile(QString fileLocation, int joystickIndex)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((joystickIndex > 0) && joysticks->contains(joystickIndex - 1))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    settings->sync();
    loadAppConfig(true);
}

void MainWindow::openJoystickStatusWindow()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QKeyDisplayDialog *dialog = new QKeyDisplayDialog(this);
    dialog->show();
}

void MainWindow::openGitHubPage()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QDesktopServices::openUrl(QUrl(PadderCommon::githubProjectPage));
}

void MainWindow::openIssuesPage()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QDesktopServices::openUrl(QUrl(PadderCommon::githubIssuesPage));
}

void MainWindow::openWikiPage()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QDesktopServices::openUrl(QUrl(PadderCommon::wikiPage));
}

void MainWindow::unloadCurrentConfig(int joystickIndex)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((joystickIndex > 0) && joysticks->contains(joystickIndex - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*> (ui->tabWidget->widget(joystickIndex - 1)); // static_cast
        if (widget != nullptr)
        {
            widget->unloadConfig();
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

void MainWindow::propogateNameDisplayStatus(bool displayNames)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyTabWidget *tabwidget = qobject_cast<JoyTabWidget*>(sender());  // static_cast
    for (int i=0; i < ui->tabWidget->count(); i++)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((joystickIndex > 0) && joysticks->contains(joystickIndex - 1))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QList<InputDevice*> *devices = new QList<InputDevice*>(joysticks->values());
    MainSettingsDialog *dialog = new MainSettingsDialog(settings, devices, this);
    connect(dialog, SIGNAL(changeLanguage(QString)), this, SLOT(changeLanguage(QString)));

    if (appWatcher != nullptr)
    {
#if defined(USE_SDL_2) && defined(Q_OS_UNIX) && defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    connect(dialog, SIGNAL(accepted()), appWatcher, SLOT(syncProfileAssignment()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkAutoProfileWatcherTimer()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(checkAutoProfileWatcherTimer()));
    appWatcher->stopTimer();
    }

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((translator != nullptr) && (appTranslator != nullptr))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
        Joystick *joystick = qobject_cast<Joystick*>(joyTab->getJoystick()); // static_cast

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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


void MainWindow::profileTrayActionTriggered(bool checked)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    // Obtaining the selected config
    QAction *action = qobject_cast<QAction*>(sender());  // static_cast
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(index)); // static_cast
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick != nullptr)
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
        Logger::LogInfo(trUtf8("Could not find controller. Exiting."));
        qApp->quit();
    }
}

void MainWindow::propogateMappingUpdate(QString mapping, InputDevice *device)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    emit mappingUpdated(mapping, device);
}

void MainWindow::testMappingUpdateNow(int index, InputDevice *device)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QWidget *tab = ui->tabWidget->widget(index);
    if (tab != nullptr)
    {
        ui->tabWidget->removeTab(index);
        delete tab;
        tab = nullptr;
    }

    JoyTabWidget *tabwidget = new JoyTabWidget(device, settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyTabWidget *tabwidget = new JoyTabWidget(device, settings, this);
    QString joytabName = device->getSDLName();
    joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
    ui->tabWidget->addTab(tabwidget, joytabName);
    tabwidget->loadDeviceSettings();
    tabwidget->refreshButtons();

    // Refresh tab text to reflect new index values.
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(ui->tabWidget->widget(i)); // static_cast
        if (tab != nullptr)
        {
            InputDevice *device = tab->getJoystick();
            QString joytabName = device->getSDLName();
            joytabName.append(" ").append(trUtf8("(%1)").arg(device->getName()));
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

  if( info != NULL ) {
    Logger::LogDebug(QObject::trUtf8("Auto-switching to profile \"%1\".").
		     arg(info->getProfileLocation()));
  } else {
    Logger::LogError(QObject::trUtf8("Auto-switching to NULL profile!"));    
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
    #if defined(Q_OS_UNIX)
    }
    #endif
#endif
}

void MainWindow::checkAutoProfileWatcherTimer()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

#if defined(USE_SDL_2) && (defined(WITH_X11) || defined(Q_OS_WIN))
    #if defined(Q_OS_UNIX)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((index > 0) && joysticks->contains(index - 1))
    {
        JoyTabWidget *widget = qobject_cast<JoyTabWidget*> (ui->tabWidget->widget(index - 1));  // static_cast
        if (widget != nullptr)
        {
            ui->tabWidget->setCurrentIndex(index - 1);
        }
    }
}

/**
 * @brief Select appropriate tab that has a device with the specified GUID.
 * @param GUID of joystick device.
 */
void MainWindow::selectControllerJoyTab(QString GUID)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (!GUID.isEmpty())
    {
        InputDevice *device = nullptr;
        QMapIterator<SDL_JoystickID, InputDevice*> deviceIter(*joysticks);
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
            ui->tabWidget->setCurrentIndex(device->getJoyNumber());
        }
    }
}

void MainWindow::changeWindowStatus()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return graphical;
}

void MainWindow::setTranslator(QTranslator *translator)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->translator = translator;
}

QTranslator* MainWindow::getTranslator() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return translator;
}

void MainWindow::setAppTranslator(QTranslator *translator)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->appTranslator = translator;
}

QTranslator* MainWindow::getAppTranslator() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return appTranslator;
}

void MainWindow::retranslateUi()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->retranslateUi(this);
}

void MainWindow::refreshTabHelperThreads()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
