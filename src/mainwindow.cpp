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
#include "advancestickassignmentdialog.h"
#include "common.h"

#ifdef USE_SDL_2
#include "gamecontrollermappingdialog.h"
#include "autoprofileinfo.h"
#endif

MainWindow::MainWindow(QHash<SDL_JoystickID, InputDevice*> *joysticks, CommandLineUtility *cmdutility, QSettings *settings, bool graphical, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    //delete ui->tab_2;
    //ui->tab_2 = 0;

    //delete ui->tab;
    //ui->tab = 0;

    this->translator = 0;
    this->cmdutility = cmdutility;
    this->graphical = graphical;
    this->settings = settings;

#ifndef USE_SDL_2
    ui->actionGameController_Mapping->setVisible(false);
    ui->actionStick_Pad_Assign->setVisible(false);
#endif

#ifdef USE_SDL_2
    this->appWatcher = new AutoProfileWatcher(settings, this);
    checkAutoProfileWatcherTimer();
#else
    this->appWatcher = 0;
#endif

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

    resize(settings->value("WindowSize", size()).toSize());
    move(settings->value("WindowPosition", pos()).toPoint());

    aboutDialog = new AboutDialog(this);

    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
    connect(ui->actionKeyValue, SIGNAL(triggered()), this, SLOT(openKeyCheckerDialog()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionProperties, SIGNAL(triggered()), this, SLOT(openJoystickStatusWindow()));
    connect(ui->actionHomePage, SIGNAL(triggered()), this, SLOT(openProjectHomePage()));
    connect(ui->actionGitHubPage, SIGNAL(triggered()), this, SLOT(openGitHubPage()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openMainSettingsDialog()));

#ifdef USE_SDL_2
    connect(ui->actionGameController_Mapping, SIGNAL(triggered()), this, SLOT(openGameControllerMappingWindow()));
    connect(appWatcher, SIGNAL(foundApplicableProfile(AutoProfileInfo*)), this, SLOT(autoprofileLoad(AutoProfileInfo*)));
    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(updateMenuOptions()));
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
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();

            setEnabled(false); // Should already be disabled. Do it again just to be sure.
        }
        else if (cmdutility->isHiddenRequested() || cmdutility->isLaunchInTrayEnabled())
        {
            // Window should already be hidden but make sure
            // to disable flashing buttons.
            hideWindow();
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

void MainWindow::fillButtons(QHash<SDL_JoystickID, InputDevice *> *joysticks)
{
    ui->stackedWidget->setCurrentIndex(0);
    removeJoyTabs();

    QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);

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
#ifdef USE_SDL_2
        connect(tabwidget, SIGNAL(mappingUpdated(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString,InputDevice*)));
#endif

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
    disconnect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(singleTrayProfileMenuShow()));

    trayIconMenu->clear();
    profileActions.clear();
    unsigned int joystickCount = joysticks->count();

    if (joystickCount > 0)
    {
        QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
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

            JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(i);
            QHash<int, QString> *configs = widget->recentConfigs();
            QHashIterator<int, QString> configIter(*configs);
            QList<QAction*> tempProfileList;
            while (configIter.hasNext())
            {
                configIter.next();
                QAction *newaction = new QAction(configIter.value(), joysticksubMenu);
                newaction->setCheckable(true);
                newaction->setChecked(false);

                if (configIter.key() == widget->getCurrentConfigIndex())
                {
                    newaction->setChecked(true);
                }

                QHash<QString, QVariant> *tempmap = new QHash<QString, QVariant> ();
                tempmap->insert(QString::number(i), QVariant (configIter.key()));
                QVariant tempvar (*tempmap);
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

            QAction *newaction = new QAction(tr("Open File"), joysticksubMenu);
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

    QIcon icon = QIcon(":/images/antimicro_trayicon.png");
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
#endif
}

void MainWindow::saveAppConfig()
{
    if (joysticks->count() > 0)
    {
        JoyTabWidget *temptabwidget = (JoyTabWidget*)ui->tabWidget->widget(0);
        settings->setValue("DisplayNames",
            temptabwidget->isDisplayingNames() ? "1" : "0");

        settings->beginGroup("Controllers");
        QStringList tempIdentifierHolder;

        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            bool prepareSave = true;

            JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(i);
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
            JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);

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
        QListIterator<QObject*> iter(ui->tabWidget->children());
        while (iter.hasNext())
        {
            JoyTabWidget *tab = static_cast<JoyTabWidget*>(iter.next());
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

void MainWindow::openMainSettingsDialog()
{
    QList<InputDevice*> *devices = new QList<InputDevice*>(joysticks->values());
    MainSettingsDialog *dialog = new MainSettingsDialog(settings, devices, this);
    //connect(dialog, SIGNAL(changeLanguage(QString)), this, SLOT(changeLanguage(QString)));

#ifdef USE_SDL_2
    connect(dialog, SIGNAL(accepted()), appWatcher, SLOT(syncProfileAssignment()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(checkAutoProfileWatcherTimer()));
#endif

    connect(dialog, SIGNAL(accepted()), this, SLOT(populateTrayIcon()));

    dialog->show();
}

void MainWindow::changeLanguage(QString language)
{
    if (translator)
    {
        qApp->removeTranslator(translator);
    #if defined(Q_OS_UNIX)
        translator->load("antimicro_" + language, QApplication::applicationDirPath().append("/../share/antimicro/translations"));
    #elif defined(Q_OS_WIN)
        translator->load("antimicro_" + language, QApplication::applicationDirPath().append("\\share\\antimicro\\translations"));
    #endif
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        delete aboutDialog;
        aboutDialog = new AboutDialog(this);
    }
}

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
                    JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);

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
        JoyTabWidget *widget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);

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

                if (!found)
                {
                    widget->loadConfigFile(info->getProfileLocation());
                }
            }
            else if (info->getGUID() == widget->getJoystick()->getStringIdentifier())
            {
                widget->loadConfigFile(info->getProfileLocation());
            }
        }
    }
}

void MainWindow::checkAutoProfileWatcherTimer()
{
    QString autoProfileActive = settings->value("AutoProfiles/AutoProfilesActive", "0").toString();
    if (autoProfileActive == "1")
    {
        appWatcher->startTimer();
    }
    else
    {
        appWatcher->stopTimer();
    }
}

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

#endif
