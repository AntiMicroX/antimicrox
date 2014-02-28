#include <QDebug>
#include <QFile>
#include <QLocalSocket>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

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
#include "common.h"

#ifdef USE_SDL_2
#include "gamecontrollermappingdialog.h"
#endif

#ifdef USE_SDL_2
MainWindow::MainWindow(QHash<SDL_JoystickID, InputDevice*> *joysticks, CommandLineUtility *cmdutility, QSettings *settings, bool graphical, QWidget *parent) :
#else
MainWindow::MainWindow(QHash<int, InputDevice*> *joysticks, CommandLineUtility *cmdutility, QSettings *settings, bool graphical, QWidget *parent) :
#endif
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    delete ui->tab_2;
    ui->tab_2 = 0;

    delete ui->tab;
    ui->tab = 0;

    this->cmdutility = cmdutility;
    this->graphical = graphical;
    this->settings = settings;
    signalDisconnect = false;
    showTrayIcon = !cmdutility->isTrayHidden() && graphical;

    this->joysticks = joysticks;

    if (showTrayIcon)
    {
        trayIconMenu = new QMenu(this);
        trayIcon = new QSystemTrayIcon(this);
        connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(refreshTrayIconMenu()));
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconClickAction(QSystemTrayIcon::ActivationReason)), Qt::DirectConnection);
    }

    // Look at flags and call setEnabled as desired; defaults to true.
    // enabled status is used to specify whether errors
    // in profile loading and saving should be
    // display in a window or written to stderr.
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

    fillButtons(joysticks);
    if (cmdutility->hasProfile())
    {
        if (cmdutility->hasControllerNumber())
        {
            loadConfigFile(cmdutility->getProfileLocation(), cmdutility->getControllerNumber());
        }
        else if (cmdutility->hasControllerID())
        {
            loadConfigFile(cmdutility->getProfileLocation(), cmdutility->hasControllerID());
        }
        else
        {
            loadConfigFile(cmdutility->getProfileLocation());
        }
    }
    else if (cmdutility->isUnloadRequested())
    {
        if (cmdutility->hasControllerNumber())
        {
            unloadCurrentConfig(cmdutility->getControllerNumber());
        }
        else if (cmdutility->hasControllerID())
        {
            unloadCurrentConfig(cmdutility->hasControllerID());
        }
        else
        {
            unloadCurrentConfig(0);
        }
    }
    else if (cmdutility->getStartSetNumber() > 0)
    {
        if (cmdutility->hasControllerNumber())
        {
            changeStartSetNumber(cmdutility->getJoyStartSetNumber(), cmdutility->getControllerNumber());
        }
        else if (cmdutility->hasControllerID())
        {
            changeStartSetNumber(cmdutility->getJoyStartSetNumber(), cmdutility->getControllerID());
        }
        else
        {
            changeStartSetNumber(cmdutility->getJoyStartSetNumber());
        }
    }

    aboutDialog = new AboutDialog(this);

    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->actionKeyValue, SIGNAL(triggered()), this, SLOT(openKeyCheckerDialog()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionProperties, SIGNAL(triggered()), this, SLOT(openJoystickStatusWindow()));
    connect(ui->actionHomePage, SIGNAL(triggered()), this, SLOT(openProjectHomePage()));
    connect(ui->actionGitHubPage, SIGNAL(triggered()), this, SLOT(openGitHubPage()));
#ifdef USE_SDL_2
    connect(ui->actionGameController_Mapping, SIGNAL(triggered()), this, SLOT(openGameControllerMappingWindow()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openMainSettingsDialog()));
#endif

    // Check flags to see if user requested for the main window and the tray icon
    // to not be displayed.
    if (graphical)
    {
        if (!cmdutility->isHiddenRequested() && (!cmdutility->isLaunchInTrayEnabled() || !QSystemTrayIcon::isSystemTrayAvailable()))
        {
            show();
        }
        else if (cmdutility->isHiddenRequested() && cmdutility->isTrayHidden())
        {
            hideWindow();
            setEnabled(false); // Should already be disabled. Do it again just to be sure.
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillButtons(InputDevice *joystick)
{
    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);
    tabwidget->fillButtons();
}

#ifdef USE_SDL_2
void MainWindow::fillButtons(QHash<SDL_JoystickID, InputDevice *> *joysticks)
#else
void MainWindow::fillButtons(QHash<int, InputDevice *> *joysticks)
#endif
{
    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

#ifdef USE_SDL_2
    QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
#else
    QHashIterator<int, InputDevice*> iter(*joysticks);
#endif

    while (iter.hasNext())
    {
        iter.next();

        InputDevice *joystick = iter.value();

        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, settings, this);
        QString joytabName = joystick->getSDLName();
        joytabName.append(" ").append(tr("(%1)").arg(joystick->getName()));
        ui->tabWidget->addTab(tabwidget, joytabName);
        tabwidget->fillButtons();
        connect(tabwidget, SIGNAL(namesDisplayChanged(bool)), this, SLOT(propogateNameDisplayStatus(bool)));

        if (showTrayIcon)
        {
            connect(tabwidget, SIGNAL(joystickConfigChanged(int)), this, SLOT(populateTrayIcon()));
        }
    }

    if (joysticks->count() > 0)
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

void MainWindow::joystickRefreshPropogate(InputDevice *joystick)
{
    emit joystickRefreshRequested(joystick);
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
    trayIconMenu->clear();

    if (joysticks->count() > 0)
    {
#ifdef USE_SDL_2
        QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
#else
        QHashIterator<int, InputDevice*> iter(*joysticks);
#endif
        int i = 0;
        while (iter.hasNext())
        {
            iter.next();
            InputDevice *current = iter.value();

            QString joytabName = current->getSDLName();
            joytabName.append(" ").append(tr("(%1)").arg(current->getName()));

            QMenu *joysticksub = trayIconMenu->addMenu(joytabName);
            JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(i);
            QHash<int, QString> *configs = widget->recentConfigs();
            QHashIterator<int, QString> iter(*configs);
            while (iter.hasNext())
            {
                iter.next();
                QAction *newaction = new QAction(iter.value(), joysticksub);
                newaction->setCheckable(true);
                newaction->setChecked(false);

                if (iter.key() == widget->getCurrentConfigIndex())
                {
                    newaction->setChecked(true);
                }
                QHash<QString, QVariant> *tempmap = new QHash<QString, QVariant> ();
                tempmap->insert(QString::number(i), QVariant (iter.key()));
                QVariant tempvar (*tempmap);
                newaction->setData(tempvar);
                joysticksub->addAction(newaction);
            }

            delete configs;
            configs = 0;

            QAction *newaction = new QAction(tr("Open File"), joysticksub);
            newaction->setIcon(QIcon::fromTheme("document-open"));
            connect(newaction, SIGNAL(triggered()), widget, SLOT(openConfigFileDialog()));
            joysticksub->addAction(newaction);

            connect(joysticksub, SIGNAL(triggered(QAction*)), this, SLOT(trayMenuChangeJoyConfig(QAction*)));
            connect(joysticksub, SIGNAL(aboutToShow()), this, SLOT(joystickTrayShow()));

            i++;
        }

        trayIconMenu->addSeparator();
    }

    hideAction = new QAction(tr("&Hide"), trayIconMenu);
    hideAction->setIcon(QIcon::fromTheme("view-restore"));
    connect(hideAction, SIGNAL(triggered()), this, SLOT(hideWindow()));
    //connect(hideAction, SIGNAL(triggered()), this, SLOT(disableFlashActions()));

    restoreAction = new QAction(tr("&Restore"), trayIconMenu);
    restoreAction->setIcon(QIcon::fromTheme("view-fullscreen"));
    //connect(restoreAction, SIGNAL(triggered()), this, SLOT(enableFlashActions()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    closeAction = new QAction(tr("&Quit"), trayIconMenu);
    closeAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));

    updateJoy = new QAction(tr("&Update Joysticks"), trayIconMenu);
    updateJoy->setIcon(QIcon::fromTheme("view-refresh"));
    connect(updateJoy, SIGNAL(triggered()), this, SLOT(startJoystickRefresh()));

    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(updateJoy);
    trayIconMenu->addAction(closeAction);

    QIcon icon = QIcon(":/images/antimicro_trayicon.png");
    //trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::quitProgram()
{
    this->close();
    qApp->quit();
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
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        ui->actionHide->setEnabled(true);
    }
    else
    {
        ui->actionHide->setEnabled(false);
    }

#ifndef USE_SDL_2
    ui->actionGameController_Mapping->setVisible(false);
    ui->actionOptions->setVisible(false);
#endif
}

void MainWindow::saveAppConfig()
{
    if (joysticks->count() > 0)
    {
        JoyTabWidget *temptabwidget = (JoyTabWidget*)ui->tabWidget->widget(0);
        //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
        settings->setValue("DisplayNames",
            temptabwidget->isDisplayingNames() ? "1" : "0");

        settings->beginGroup("Controllers");
        //settings->remove("");
        QStringList tempGUIDHolder;
        QStringList tempNameHolder;

        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            bool prepareSave = true;

            JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(i);
            InputDevice *device = tabwidget->getJoystick();

            // Do not allow multi-controller adapters to overwrite each
            // others recent config file list. Use first controller
            // detected to save recent config list. Flag controller string
            // afterwards.
            if (!device->getGUIDString().isEmpty())
            {
                if (tempGUIDHolder.contains(device->getGUIDString()))
                {
                    prepareSave = false;
                }
                else
                {
                    tempGUIDHolder.append(device->getGUIDString());
                }
            }
            else if (!device->getSDLName().isEmpty())
            {
                if (tempNameHolder.contains(device->getSDLName()))
                {
                    prepareSave = false;
                }
                else
                {
                    tempNameHolder.append(device->getSDLName());
                }
            }

            if (prepareSave)
            {
                tabwidget->saveSettings();
            }
        }

        settings->endGroup();
    }
}

void MainWindow::loadAppConfig(bool forceRefresh)
{
    //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(i);
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
        }

        QList<JoyAxisWidget*> list2 = ui->tabWidget->widget(i)->findChildren<JoyAxisWidget*>();
        QListIterator<JoyAxisWidget*> iter2(list2);
        while (iter2.hasNext())
        {
            JoyAxisWidget *axisWidget = iter2.next();
            axisWidget->enableFlashes();
        }

        QList<JoyControlStickPushButton*> list3 = ui->tabWidget->widget(i)->findChildren<JoyControlStickPushButton*>();
        QListIterator<JoyControlStickPushButton*> iter3(list3);
        while (iter3.hasNext())
        {
            JoyControlStickPushButton *stickWidget = iter3.next();
            stickWidget->enableFlashes();
        }

        QList<JoyDPadButtonWidget*> list4 = ui->tabWidget->widget(i)->findChildren<JoyDPadButtonWidget*>();
        QListIterator<JoyDPadButtonWidget*> iter4(list4);
        while (iter4.hasNext())
        {
            JoyDPadButtonWidget *dpadWidget = iter4.next();
            dpadWidget->enableFlashes();
        }

        QList<JoyControlStickButtonPushButton*> list6 = ui->tabWidget->widget(i)->findChildren<JoyControlStickButtonPushButton*>();
        QListIterator<JoyControlStickButtonPushButton*> iter6(list6);
        while (iter6.hasNext())
        {
            JoyControlStickButtonPushButton *stickButtonWidget = iter6.next();
            stickButtonWidget->enableFlashes();
        }

        QList<DPadPushButton*> list7 = ui->tabWidget->widget(i)->findChildren<DPadPushButton*>();
        QListIterator<DPadPushButton*> iter7(list7);
        while (iter7.hasNext())
        {
            DPadPushButton *dpadWidget = iter7.next();
            dpadWidget->enableFlashes();
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

void MainWindow::trayMenuChangeJoyConfig(QAction *action)
{
    // Obtaining the selected config
    QHash<QString, QVariant> tempmap = action->data().toHash();
    QHashIterator<QString, QVariant> iter(tempmap);
    while (iter.hasNext())
    {
        iter.next();

        // Fetching indicies and tab associated with the current joypad
        int joyindex = iter.key().toInt();
        int configindex = iter.value().toInt();
        JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);

        // Checking if the selected config has been disabled by the change (action->isChecked() represents the state of the checkbox AFTER the click)
        if (!action->isChecked())
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

void MainWindow::joystickTrayShow()
{
    QMenu *tempmenu = (QMenu*) sender();
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
            JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);

            if (configindex == widget->getCurrentConfigIndex())
            {
                action->setChecked(true);
            }
        }
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    // Check if hideEvent has been processed
    if (signalDisconnect)
    {
        // Restore flashing buttons
        enableFlashActions();
        signalDisconnect = false;

        // Only needed if hidden with the system tray enabled
        if (QSystemTrayIcon::isSystemTrayAvailable() && showTrayIcon)
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
        }
    }

    QMainWindow::showEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *e = (QWindowStateChangeEvent*)event;
        if (e->oldState() != Qt::WindowMinimized && isMinimized())
        {
            if (QSystemTrayIcon::isSystemTrayAvailable() && showTrayIcon)
            {
                hideWindow();
            }
            else
            {
                disableFlashActions();
                signalDisconnect = true;
            }
        }
        else if (e->oldState() == Qt::WindowMinimized)
        {
            if (signalDisconnect)
            {
                // Restore flashing buttons
                enableFlashActions();
                signalDisconnect = false;
            }
        }
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
        JoyTabWidget *widget = static_cast<JoyTabWidget*> (ui->tabWidget->widget(joystickIndex-1));
        if (widget)
        {
            widget->loadConfigFile(fileLocation);
        }
    }
    else if (joystickIndex <= 0)
    {
        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *widget = static_cast<JoyTabWidget*> (ui->tabWidget->widget(i));
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
        QListIterator<QObject*> iter(ui->tabWidget->children());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = static_cast<JoyTabWidget*>(iter.next());
            if (tab)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getGUIDString())
                {
                    tab->loadConfigFile(fileLocation);
                }
                else if (controllerID == tempdevice->getSDLName())
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

void MainWindow::startLocalServer()
{
    localServer = new QLocalServer(this);
    QLocalServer::removeServer(PadderCommon::localSocketKey);
    localServer->setMaxPendingConnections(1);
    if (!localServer->listen(PadderCommon::localSocketKey))
    {
        QTextStream errorstream(stderr);
        QString message("Could not start signal server. Profiles cannot be reloaded\n");
        message.append("from command-line");
        errorstream << tr(message.toStdString().c_str()) << endl;
    }
    else
    {
        connect(localServer, SIGNAL(newConnection()), this, SLOT(handleOutsideConnection()));
    }
}

void MainWindow::handleOutsideConnection()
{
    QLocalSocket *socket = localServer->nextPendingConnection();
    if (socket)
    {
        connect(socket, SIGNAL(disconnected()), this, SLOT(handleSocketDisconnect()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    }
}

void MainWindow::handleSocketDisconnect()
{
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

void MainWindow::openProjectHomePage()
{
    QDesktopServices::openUrl(QUrl(PadderCommon::projectHomePage));
}

void MainWindow::openGitHubPage()
{
    QDesktopServices::openUrl(QUrl(PadderCommon::githubProjectPage));
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
        QListIterator<QObject*> iter(ui->tabWidget->children());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = static_cast<JoyTabWidget*>(iter.next());
            if (tab)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getGUIDString())
                {
                    tab->unloadConfig();
                }
                else if (controllerID == tempdevice->getSDLName())
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
        QListIterator<QObject*> iter(ui->tabWidget->children());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = static_cast<JoyTabWidget*>(iter.next());
            if (tab)
            {
                InputDevice *tempdevice = tab->getJoystick();
                if (controllerID == tempdevice->getGUIDString())
                {
                    tab->changeCurrentSet(startSetNumber);
                }
                else if (controllerID == tempdevice->getSDLName())
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

#ifdef USE_SDL_2
void MainWindow::openGameControllerMappingWindow()
{
    int index = ui->tabWidget->currentIndex();
    if (index >= 0)
    {
        JoyTabWidget *joyTab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(index));
        InputDevice *joystick = joyTab->getJoystick();
        if (joystick)
        {
            GameControllerMappingDialog *dialog = new GameControllerMappingDialog(joystick, settings, this);
            dialog->show();
            connect(dialog, SIGNAL(mappingUpdate(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString, InputDevice*)));
        }
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
    tabwidget->fillButtons();
    ui->tabWidget->setCurrentIndex(index);
}

void MainWindow::removeJoyTab(SDL_JoystickID deviceID)
{
    bool found = false;
    for (int i=0; i < ui->tabWidget->count() && !found; i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(ui->tabWidget->widget(i));
        if (tab && deviceID == tab->getJoystick()->getSDLJoystickID())
        {
            ui->tabWidget->disableFlashes(tab->getJoystick());
            ui->tabWidget->removeTab(i);
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
    tabwidget->fillButtons();

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
    if (showTrayIcon)
    {
        connect(tabwidget, SIGNAL(joystickConfigChanged(int)), this, SLOT(populateTrayIcon()));
        trayIcon->hide();
        populateTrayIcon();
        trayIcon->show();
    }

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::openMainSettingsDialog()
{
    MainSettingsDialog *dialog = new MainSettingsDialog(settings, this);
    dialog->show();
}

#endif
