#include <QDebug>
#include <SDL/SDL.h>
#include <typeinfo>
#include <QFile>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "joytabwidget.h"
#include "common.h"

MainWindow::MainWindow(QHash<int, Joystick*> *joysticks, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->tab_2->deleteLater();
    ui->tab->deleteLater();

    this->joysticks = joysticks;

    populateTrayIcon();
    trayIcon->show();

    connect(ui->menuOptions, SIGNAL(aboutToShow()), this, SLOT(mainMenuChange()));
}

MainWindow::~MainWindow()
{
    delete ui;
    this->deleteLater();
}

void MainWindow::fillButtons(Joystick *joystick)
{
    int joyindex = joystick->getJoyNumber();
    JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(joyindex);
    tabwidget->fillButtons();
}

void MainWindow::fillButtons(QHash<int, Joystick *> *joysticks)
{
    for (int i=0; i < joysticks->count(); i++)
    {
        Joystick *joystick = joysticks->value(i);
        if (ui->tabWidget->widget(i) != 0)
        {
            ui->tabWidget->widget(i)->deleteLater();
        }

        JoyTabWidget *tabwidget = new JoyTabWidget(joystick, this);
        tabwidget->fillButtons();
        ui->tabWidget->addTab(tabwidget, QString("Joystick %1").arg(joystick->getRealJoyNumber()));
        connect(tabwidget, SIGNAL(joystickRefreshRequested(Joystick*)), this, SLOT(joystickRefreshPropogate(Joystick*)));
    }

    if (joysticks->count() > 0)
    {
        loadAppConfig();

        ui->tabWidget->setCurrentIndex(0);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::joystickRefreshPropogate(Joystick *joystick)
{
    emit joystickRefreshRequested(joystick);
}

// Intermediate slot to be used in Form Designer
void MainWindow::startJoystickRefresh()
{
    ui->stackedWidget->setCurrentIndex(0);

    /*for (int i=0; i < ui->tabWidget->count(); i++)
    {
        QLayout* old_layout = ui->tabWidget->widget(i)->findChild<QLayout*>("gridLayout");
        QWidget* child = 0;

        while (old_layout && old_layout->count() > 0)
        {
            child = old_layout->takeAt(0)->widget();
            old_layout->removeWidget (child);
            delete child;
            child = 0;
        }
    }*/

    emit joystickRefreshRequested();
}

void MainWindow::populateTrayIcon()
{
    QMenu *trayIconMenu = new QMenu(this);

    hideAction = new QAction(tr("Hide"), this);
    connect(hideAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect(hideAction, SIGNAL(triggered()), this, SLOT(disableFlashActions()));

    restoreAction = new QAction(tr("Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(enableFlashActions()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    closeAction = new QAction(tr("Quit"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));

    updateJoy = new QAction(tr("Update Joysticks"), this);
    connect(updateJoy, SIGNAL(triggered()), this, SLOT(startJoystickRefresh()));

    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(updateJoy);
    trayIconMenu->addAction(closeAction);

    QIcon icon = QIcon(":/images/joystick_icon.png");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(refreshTrayIconMenu()));
    //connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(refreshTrayIconMenu(QSystemTrayIcon::ActivationReason)), Qt::DirectConnection);
}

void MainWindow::quitProgram()
{
    this->close();
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
}

void MainWindow::saveAppConfig()
{
    if (joysticks->count() > 0)
    {
        QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
        settings.clear();
        settings.beginGroup("Controllers");

        for (int i=0; i < ui->tabWidget->count(); i++)
        {
            JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(i);
            tabwidget->saveSettings(&settings);
        }

        settings.endGroup();
    }
}

void MainWindow::loadAppConfig()
{
    QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    for (int i=0; i < ui->tabWidget->count(); i++)
    {
        JoyTabWidget *tabwidget = (JoyTabWidget*)ui->tabWidget->widget(i);
        tabwidget->loadSettings(&settings);
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
    }
}

// Intermediate slot used in Design mode
void MainWindow::hideWindow()
{
    hide();
    disableFlashActions();
}

// Intermediate slot used in Design mode
void MainWindow::restoreWindow()
{
    enableFlashActions();
    show();
}
