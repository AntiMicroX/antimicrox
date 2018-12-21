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
#include <QCoreApplication>
#include <QLayoutItem>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextStream>
#include <QStringListIterator>
#include <QMenu>

#include "joytabwidget.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "buttoneditdialog.h"
#include "advancestickassignmentdialog.h"
#include "quicksetdialog.h"
#include "extraprofilesettingsdialog.h"
#include "setnamesdialog.h"
#include "stickpushbuttongroup.h"
#include "dpadpushbuttongroup.h"
#include "common.h"

#ifdef USE_SDL_2
#include "gamecontroller/gamecontroller.h"
#include "gamecontrollermappingdialog.h"
#endif

JoyTabWidget::JoyTabWidget(InputDevice *joystick, AntiMicroSettings *settings, QWidget *parent) :
    QWidget(parent),
    tabHelper(joystick)
{
    this->joystick = joystick;
    this->settings = settings;

    tabHelper.moveToThread(joystick->thread());

    comboBoxIndex = 0;
    hideEmptyButtons = false;

    verticalLayout = new QVBoxLayout (this);
    verticalLayout->setContentsMargins(4, 4, 4, 4);

    configHorizontalLayout = new QHBoxLayout();
    configBox = new QComboBox(this);
    configBox->addItem(tr("<New>"), "");

    configBox->setObjectName(QString::fromUtf8("configBox"));
    configBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    configHorizontalLayout->addWidget(configBox);
    spacer1 = new QSpacerItem(30, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    configHorizontalLayout->addItem(spacer1);

    removeButton = new QPushButton(tr("Remove"), this);
    removeButton->setObjectName(QString::fromUtf8("removeButton"));
    removeButton->setToolTip(tr("Remove configuration from recent list."));
    //removeButton->setFixedWidth(100);
    removeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    removeButton->setIcon(QIcon::fromTheme("edit-clear-list"));
    configHorizontalLayout->addWidget(removeButton);

    loadButton = new QPushButton(tr("Load"), this);
    loadButton->setObjectName(QString::fromUtf8("loadButton"));
    loadButton->setToolTip(tr("Load configuration file."));
    //loadButton->setFixedWidth(100);
    loadButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    loadButton->setIcon(QIcon::fromTheme("document-open"));
    configHorizontalLayout->addWidget(loadButton);

    saveButton = new QPushButton(tr("Save"), this);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    saveButton->setToolTip(tr("Save changes to configuration file."));
    //saveButton->setFixedWidth(100);
    saveButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    saveButton->setIcon(QIcon::fromTheme("document-save"));
    configHorizontalLayout->addWidget(saveButton);
    //configHorizontalLayout->setSpacing(-1);

    saveAsButton = new QPushButton(tr("Save As"), this);
    saveAsButton->setObjectName(QString::fromUtf8("saveAsButton"));
    saveAsButton->setToolTip(tr("Save changes to a new configuration file."));
    //saveAsButton->setFixedWidth(100);
    saveAsButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    saveAsButton->setIcon(QIcon::fromTheme("document-save-as"));
    configHorizontalLayout->addWidget(saveAsButton);

    verticalLayout->addLayout(configHorizontalLayout);
    verticalLayout->setStretchFactor(configHorizontalLayout, 1);

    spacer2 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer2);

    verticalSpacer_2 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

    verticalLayout->addItem(verticalSpacer_2);


    stackedWidget_2 = new QStackedWidget(this);
    stackedWidget_2->setObjectName(QString::fromUtf8("stackedWidget_2"));

    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));

    QVBoxLayout *tempVBoxLayout = new QVBoxLayout(page);
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setObjectName(QString::fromUtf8("scrollArea1"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //sizePolicy.setHorizontalStretch(0);
    //sizePolicy.setVerticalStretch(0);
    scrollArea->setSizePolicy(sizePolicy);
    scrollArea->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents1 = new QWidget();
    scrollAreaWidgetContents1->setObjectName(QString::fromUtf8("scrollAreaWidgetContents1"));

    gridLayout = new QGridLayout(scrollAreaWidgetContents1);
    gridLayout->setSpacing(4);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

    scrollArea->setWidget(scrollAreaWidgetContents1);
    tempVBoxLayout->addWidget(scrollArea);
    stackedWidget_2->addWidget(page);

    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));

    tempVBoxLayout = new QVBoxLayout(page_2);
    QScrollArea *scrollArea2 = new QScrollArea();
    scrollArea2->setObjectName(QString::fromUtf8("scrollArea2"));
    scrollArea2->setSizePolicy(sizePolicy);
    scrollArea2->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents2 = new QWidget();
    scrollAreaWidgetContents2->setObjectName(QString::fromUtf8("scrollAreaWidgetContents2"));

    gridLayout2 = new QGridLayout(scrollAreaWidgetContents2);
    gridLayout2->setSpacing(4);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));

    scrollArea2->setWidget(scrollAreaWidgetContents2);
    tempVBoxLayout->addWidget(scrollArea2);
    stackedWidget_2->addWidget(page_2);


    page_3 = new QWidget();
    page_3->setObjectName(QString::fromUtf8("page_3"));

    tempVBoxLayout = new QVBoxLayout(page_3);
    QScrollArea *scrollArea3 = new QScrollArea();
    scrollArea3->setObjectName(QString::fromUtf8("scrollArea3"));
    scrollArea3->setSizePolicy(sizePolicy);
    scrollArea3->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents3 = new QWidget();
    scrollAreaWidgetContents3->setObjectName(QString::fromUtf8("scrollAreaWidgetContents3"));

    gridLayout3 = new QGridLayout(scrollAreaWidgetContents3);
    gridLayout3->setSpacing(4);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));

    scrollArea3->setWidget(scrollAreaWidgetContents3);
    tempVBoxLayout->addWidget(scrollArea3);
    stackedWidget_2->addWidget(page_3);


    page_4 = new QWidget();
    page_4->setObjectName(QString::fromUtf8("page_4"));

    tempVBoxLayout = new QVBoxLayout(page_4);
    QScrollArea *scrollArea4 = new QScrollArea();
    scrollArea4->setObjectName(QString::fromUtf8("scrollArea4"));
    scrollArea4->setSizePolicy(sizePolicy);
    scrollArea4->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents4 = new QWidget();
    scrollAreaWidgetContents4->setObjectName(QString::fromUtf8("scrollAreaWidgetContents4"));

    gridLayout4 = new QGridLayout(scrollAreaWidgetContents4);
    gridLayout4->setSpacing(4);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));

    scrollArea4->setWidget(scrollAreaWidgetContents4);
    tempVBoxLayout->addWidget(scrollArea4);
    stackedWidget_2->addWidget(page_4);

    page_5 = new QWidget();
    page_5->setObjectName(QString::fromUtf8("page_5"));

    tempVBoxLayout = new QVBoxLayout(page_5);
    QScrollArea *scrollArea5 = new QScrollArea();
    scrollArea5->setObjectName(QString::fromUtf8("scrollArea5"));
    scrollArea5->setSizePolicy(sizePolicy);
    scrollArea5->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents5 = new QWidget();
    scrollAreaWidgetContents5->setObjectName(QString::fromUtf8("scrollAreaWidgetContents5"));

    gridLayout5 = new QGridLayout(scrollAreaWidgetContents5);
    gridLayout5->setSpacing(4);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));

    scrollArea5->setWidget(scrollAreaWidgetContents5);
    tempVBoxLayout->addWidget(scrollArea5);
    stackedWidget_2->addWidget(page_5);

    page_6 = new QWidget();
    page_6->setObjectName(QString::fromUtf8("page_6"));

    tempVBoxLayout = new QVBoxLayout(page_6);
    QScrollArea *scrollArea6 = new QScrollArea();
    scrollArea6->setObjectName(QString::fromUtf8("scrollArea6"));
    scrollArea6->setSizePolicy(sizePolicy);
    scrollArea6->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents6 = new QWidget();
    scrollAreaWidgetContents6->setObjectName(QString::fromUtf8("scrollAreaWidgetContents6"));

    gridLayout6 = new QGridLayout(scrollAreaWidgetContents6);
    gridLayout6->setSpacing(4);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));

    scrollArea6->setWidget(scrollAreaWidgetContents6);
    tempVBoxLayout->addWidget(scrollArea6);
    stackedWidget_2->addWidget(page_6);

    page_7 = new QWidget();
    page_7->setObjectName(QString::fromUtf8("page_7"));

    tempVBoxLayout = new QVBoxLayout(page_7);
    QScrollArea *scrollArea7 = new QScrollArea();
    scrollArea7->setObjectName(QString::fromUtf8("scrollArea7"));
    scrollArea7->setSizePolicy(sizePolicy);
    scrollArea7->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents7 = new QWidget();
    scrollAreaWidgetContents7->setObjectName(QString::fromUtf8("scrollAreaWidgetContents7"));

    gridLayout7 = new QGridLayout(scrollAreaWidgetContents7);
    gridLayout7->setSpacing(4);
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));

    scrollArea7->setWidget(scrollAreaWidgetContents7);
    tempVBoxLayout->addWidget(scrollArea7);
    stackedWidget_2->addWidget(page_7);

    page_8 = new QWidget();
    page_8->setObjectName(QString::fromUtf8("page_8"));

    tempVBoxLayout = new QVBoxLayout(page_8);
    QScrollArea *scrollArea8 = new QScrollArea();
    scrollArea8->setObjectName(QString::fromUtf8("scrollArea8"));
    scrollArea8->setSizePolicy(sizePolicy);
    scrollArea8->setWidgetResizable(true);

    QWidget *scrollAreaWidgetContents8 = new QWidget();
    scrollAreaWidgetContents8->setObjectName(QString::fromUtf8("scrollAreaWidgetContents8"));

    gridLayout8 = new QGridLayout(scrollAreaWidgetContents8);
    gridLayout8->setSpacing(4);
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));

    scrollArea8->setWidget(scrollAreaWidgetContents8);
    tempVBoxLayout->addWidget(scrollArea8);
    stackedWidget_2->addWidget(page_8);

    verticalLayout->addWidget(stackedWidget_2);

    verticalSpacer_3 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    verticalLayout->addItem(verticalSpacer_3);


    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));

    setsMenuButton = new QPushButton(tr("Sets"), this);
    QMenu *setMenu = new QMenu(setsMenuButton);
    copySetMenu = new QMenu(tr("Copy from Set"), setMenu);
    QAction *setSettingsAction = new QAction(tr("Settings"), setMenu);
    connect(setSettingsAction, SIGNAL(triggered()), this, SLOT(showSetNamesDialog()));
    setMenu->addAction(setSettingsAction);
    setMenu->addMenu(copySetMenu);
    setMenu->addSeparator();

    refreshCopySetActions();

    setAction1 = new QAction(tr("Set 1"), setMenu);
    connect(setAction1, SIGNAL(triggered()), this, SLOT(changeSetOne()));
    setMenu->addAction(setAction1);

    setAction2 = new QAction(tr("Set 2"), setMenu);
    connect(setAction2, SIGNAL(triggered()), this, SLOT(changeSetTwo()));
    setMenu->addAction(setAction2);

    setAction3 = new QAction(tr("Set 3"), setMenu);
    connect(setAction3, SIGNAL(triggered()), this, SLOT(changeSetThree()));
    setMenu->addAction(setAction3);

    setAction4 = new QAction(tr("Set 4"), setMenu);
    connect(setAction4, SIGNAL(triggered()), this, SLOT(changeSetFour()));
    setMenu->addAction(setAction4);

    setAction5 = new QAction(tr("Set 5"), setMenu);
    connect(setAction5, SIGNAL(triggered()), this, SLOT(changeSetFive()));
    setMenu->addAction(setAction5);

    setAction6 = new QAction(tr("Set 6"), setMenu);
    connect(setAction6, SIGNAL(triggered()), this, SLOT(changeSetSix()));
    setMenu->addAction(setAction6);

    setAction7 = new QAction(tr("Set 7"), setMenu);
    connect(setAction7, SIGNAL(triggered()), this, SLOT(changeSetSeven()));
    setMenu->addAction(setAction7);

    setAction8 = new QAction(tr("Set 8"), setMenu);
    connect(setAction8, SIGNAL(triggered()), this, SLOT(changeSetEight()));
    setMenu->addAction(setAction8);

    setsMenuButton->setMenu(setMenu);
    horizontalLayout_2->addWidget(setsMenuButton);

    setPushButton1 = new QPushButton("1", this);
    setPushButton1->setObjectName(QString::fromUtf8("setPushButton1"));
    setPushButton1->setProperty("setActive", true);

    horizontalLayout_2->addWidget(setPushButton1);

    setPushButton2 = new QPushButton("2", this);
    setPushButton2->setObjectName(QString::fromUtf8("setPushButton2"));
    setPushButton2->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton2);

    setPushButton3 = new QPushButton("3", this);
    setPushButton3->setObjectName(QString::fromUtf8("setPushButton3"));
    setPushButton3->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton3);

    setPushButton4 = new QPushButton("4", this);
    setPushButton4->setObjectName(QString::fromUtf8("setPushButton4"));
    setPushButton4->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton4);

    setPushButton5 = new QPushButton("5", this);
    setPushButton5->setObjectName(QString::fromUtf8("setPushButton5"));
    setPushButton5->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton5);

    setPushButton6 = new QPushButton("6", this);
    setPushButton6->setObjectName(QString::fromUtf8("setPushButton6"));
    setPushButton6->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton6);

    setPushButton7 = new QPushButton("7", this);
    setPushButton7->setObjectName(QString::fromUtf8("setPushButton7"));
    setPushButton7->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton7);

    setPushButton8 = new QPushButton("8", this);
    setPushButton8->setObjectName(QString::fromUtf8("setPushButton8"));
    setPushButton8->setProperty("setActive", false);

    horizontalLayout_2->addWidget(setPushButton8);

    refreshSetButtons();

    verticalLayout->addLayout(horizontalLayout_2);

    spacer3 = new QSpacerItem(20, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addItem(spacer3);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setSpacing(6);
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));

    stickAssignPushButton = new QPushButton(tr("Stick/Pad Assign"), this);
    stickAssignPushButton->setObjectName(QString::fromUtf8("stickAssignPushButton"));
    QIcon icon7(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
    stickAssignPushButton->setIcon(icon7);
    horizontalLayout_3->addWidget(stickAssignPushButton);

    gameControllerMappingPushButton = new QPushButton(tr("Controller Mapping"), this);
    gameControllerMappingPushButton->setObjectName(QString::fromUtf8("gameControllerMappingPushButton"));
    gameControllerMappingPushButton->setIcon(QIcon::fromTheme("games-config-options"));
    gameControllerMappingPushButton->setEnabled(false);
    gameControllerMappingPushButton->setVisible(false);
    horizontalLayout_3->addWidget(gameControllerMappingPushButton);

    quickSetPushButton = new QPushButton(tr("Quick Set"), this);
    quickSetPushButton->setObjectName(QString::fromUtf8("quickSetPushButton"));
    horizontalLayout_3->addWidget(quickSetPushButton);

    QSpacerItem *horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_3->addItem(horizontalSpacer_2);

    namesPushButton = new QPushButton(tr("Names"), this);
    namesPushButton->setObjectName(QString::fromUtf8("namesPushButton"));
    namesPushButton->setToolTip(tr("Toggle button name displaying."));
    namesPushButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    namesPushButton->setIcon(QIcon::fromTheme("text-field"));
    horizontalLayout_3->addWidget(namesPushButton);

    delayButton = new QPushButton(tr("Pref"), this);
    delayButton->setObjectName(QString::fromUtf8("delayButton"));
    delayButton->setToolTip(tr("Change global profile settings."));
    delayButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    horizontalLayout_3->addWidget(delayButton);

    resetButton = new QPushButton(tr("Reset"), this);
    resetButton->setObjectName(QString::fromUtf8("resetButton"));
    resetButton->setToolTip(tr("Revert changes to the configuration. Reload configuration file."));
    resetButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    resetButton->setIcon(QIcon::fromTheme("document-revert"));
    //verticalLayout->addWidget(resetButton, 0, Qt::AlignRight);
    horizontalLayout_3->addWidget(resetButton);

    verticalLayout->addLayout(horizontalLayout_3);

    displayingNames = false;

#ifdef USE_SDL_2
    stickAssignPushButton->setEnabled(false);
    stickAssignPushButton->setVisible(false);

    gameControllerMappingPushButton->setEnabled(true);
    gameControllerMappingPushButton->setVisible(true);

#endif

#ifdef Q_OS_WIN
    deviceKeyRepeatSettings();
#endif

    checkHideEmptyOption();

    connect(loadButton, SIGNAL(clicked()), this, SLOT(openConfigFileDialog()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfigFile()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetJoystick()));
    connect(namesPushButton, SIGNAL(clicked()), this, SLOT(toggleNames()));

    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAsConfig()));
    connect(delayButton, SIGNAL(clicked()), this, SLOT(showKeyDelayDialog()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeConfig()));
    connect(setPushButton1, SIGNAL(clicked()), this, SLOT(changeSetOne()));
    connect(setPushButton2, SIGNAL(clicked()), this, SLOT(changeSetTwo()));
    connect(setPushButton3, SIGNAL(clicked()), this, SLOT(changeSetThree()));
    connect(setPushButton4, SIGNAL(clicked()), this, SLOT(changeSetFour()));
    connect(setPushButton5, SIGNAL(clicked()), this, SLOT(changeSetFive()));
    connect(setPushButton6, SIGNAL(clicked()), this, SLOT(changeSetSix()));
    connect(setPushButton7, SIGNAL(clicked()), this, SLOT(changeSetSeven()));
    connect(setPushButton8, SIGNAL(clicked()), this, SLOT(changeSetEight()));

    connect(stickAssignPushButton, SIGNAL(clicked()), this, SLOT(showStickAssignmentDialog()));
#ifdef USE_SDL_2
    connect(gameControllerMappingPushButton, SIGNAL(clicked()), this, SLOT(openGameControllerMappingWindow()));
#endif

    connect(quickSetPushButton, SIGNAL(clicked()), this, SLOT(showQuickSetDialog()));
    connect(this, SIGNAL(joystickConfigChanged(int)), this, SLOT(refreshSetButtons()));
    connect(this, SIGNAL(joystickConfigChanged(int)), this, SLOT(refreshCopySetActions()));
    connect(joystick, SIGNAL(profileUpdated()), this, SLOT(displayProfileEditNotification()));

    connect(joystick, SIGNAL(requestProfileLoad(QString)), this, SLOT(loadConfigFile(QString)), Qt::QueuedConnection);

    reconnectCheckUnsavedEvent();
    reconnectMainComboBoxEvents();
}

void JoyTabWidget::openConfigFileDialog()
{
    settings->getLock()->lock();

    int numberRecentProfiles = settings->value("NumberRecentProfiles", DEFAULTNUMBERPROFILES).toInt();
    QString lookupDir = PadderCommon::preferredProfileDir(settings);

    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, tr("Config Files (*.amgp *.xml)"));

    settings->getLock()->unlock();

    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (searchIndex == -1)
        {
            if (numberRecentProfiles > 0 && configBox->count() == numberRecentProfiles + 1)
            {
                configBox->removeItem(numberRecentProfiles);
            }

            configBox->insertItem(1, PadderCommon::getProfileName(fileinfo), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            saveDeviceSettings();
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else
        {
            configBox->setCurrentIndex(searchIndex);
            saveDeviceSettings();
            emit joystickConfigChanged(joystick->getJoyNumber());
        }

        QString outputFilename = fileinfo.absoluteDir().absolutePath();

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
        if (fileinfo.absoluteDir().isAbsolute())
        {
            QDir tempDir = fileinfo.dir();
            tempDir.cdUp();
            if (tempDir.path() == qApp->applicationDirPath())
            {
                outputFilename = QString("%1/").arg(fileinfo.dir().dirName());
            }
        }
#endif

        settings->getLock()->lock();

        settings->setValue("LastProfileDir", outputFilename);
        settings->sync();

        settings->getLock()->unlock();
    }
}

/**
 * @brief Create and render all push buttons corresponding to joystick
 *     controls for all sets.
 */
void JoyTabWidget::fillButtons()
{
    joystick->establishPropertyUpdatedConnection();
    connect(joystick, SIGNAL(setChangeActivated(int)), this, SLOT(changeCurrentSet(int)), Qt::QueuedConnection);

    for (int i=0; i < Joystick::NUMBER_JOYSETS; i++)
    {
        SetJoystick *currentSet = joystick->getSetJoystick(i);
        fillSetButtons(currentSet);
    }

    refreshCopySetActions();
}

void JoyTabWidget::showButtonDialog()
{
    JoyButtonWidget *buttonWidget = static_cast<JoyButtonWidget*>(sender());
    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, this);
    dialog->show();
}

void JoyTabWidget::showAxisDialog()
{
    JoyAxisWidget *axisWidget = static_cast<JoyAxisWidget*>(sender());
    JoyAxis *axis = axisWidget->getAxis();

    axisDialog = new AxisEditDialog (axis, this);
    axisDialog->show();
}

void JoyTabWidget::saveConfigFile()
{
    int index = configBox->currentIndex();

    settings->getLock()->lock();

    int numberRecentProfiles = settings->value("NumberRecentProfiles", DEFAULTNUMBERPROFILES).toInt();
    QString filename;
    if (index == 0)
    {
        QString lookupDir = PadderCommon::preferredProfileDir(settings);
        settings->getLock()->unlock();
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), lookupDir, tr("Config File (*.%1.amgp)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;

            QFileInfo fileinfo(filename);
            QString deviceTypeName = joystick->getXmlName();
            QString fileSuffix = deviceTypeName.append(".amgp");
            if (fileinfo.suffix() != "xml" && fileinfo.suffix() != "amgp")
            {
                filename = filename.append(".").append(fileSuffix);
            }
        }
    }
    else
    {
        settings->getLock()->unlock();
        filename = configBox->itemData(index).toString();
    }

    if (!filename.isEmpty())
    {
        //PadderCommon::inputDaemonMutex.lock();

        QFileInfo fileinfo(filename);

        QMetaObject::invokeMethod(&tabHelper, "writeConfigFile", Qt::BlockingQueuedConnection,
                                  Q_ARG(QString, fileinfo.absoluteFilePath()));
        XMLConfigWriter *writer = tabHelper.getWriter();

        /*XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);
        */

        //PadderCommon::inputDaemonMutex.unlock();

        if (writer->hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(writer->getErrorString());
            msg.setModal(true);
            msg.exec();
        }
        else if (writer->hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << writer->getErrorString() << endl;
        }
        else
        {
            int existingIndex = configBox->findData(fileinfo.absoluteFilePath());

            if (existingIndex == -1)
            {
                //PadderCommon::inputDaemonMutex.lock();

                if (numberRecentProfiles > 0 && configBox->count() == numberRecentProfiles+1)
                {
                    configBox->removeItem(numberRecentProfiles);
                }

                joystick->revertProfileEdited();
                QString tempProfileName = PadderCommon::getProfileName(fileinfo);
                if (!joystick->getProfileName().isEmpty())
                {
                    oldProfileName = joystick->getProfileName();
                    tempProfileName = oldProfileName;
                }

                disconnectCheckUnsavedEvent();
                disconnectMainComboBoxEvents();

                configBox->insertItem(1, tempProfileName, fileinfo.absoluteFilePath());

                reconnectCheckUnsavedEvent();
                reconnectMainComboBoxEvents();

                configBox->setCurrentIndex(1);
                saveDeviceSettings(true);

                //PadderCommon::inputDaemonMutex.unlock();

                emit joystickConfigChanged(joystick->getJoyNumber());
            }
            else
            {
                //PadderCommon::inputDaemonMutex.lock();

                joystick->revertProfileEdited();
                if (!joystick->getProfileName().isEmpty())
                {
                    oldProfileName = joystick->getProfileName();
                }

                configBox->setItemIcon(existingIndex, QIcon());
                saveDeviceSettings(true);

                //PadderCommon::inputDaemonMutex.unlock();

                emit joystickConfigChanged(joystick->getJoyNumber());
            }
        }
    }
}

void JoyTabWidget::resetJoystick()
{
    int currentIndex = configBox->currentIndex();
    if (currentIndex != 0)
    {
        QString filename = configBox->itemData(currentIndex).toString();

        removeCurrentButtons();

        QMetaObject::invokeMethod(&tabHelper, "readConfigFileWithRevert", Qt::BlockingQueuedConnection,
                                  Q_ARG(QString, filename));

        fillButtons();
        refreshSetButtons();
        refreshCopySetActions();

        XMLConfigReader *reader = tabHelper.getReader();
        if (!reader->hasError())
        {
            configBox->setItemIcon(currentIndex, QIcon());

            QString tempProfileName;
            if (!joystick->getProfileName().isEmpty())
            {
                tempProfileName = joystick->getProfileName();
                configBox->setItemText(currentIndex, tempProfileName);
            }
            else
            {
                tempProfileName = oldProfileName;
                configBox->setItemText(currentIndex, oldProfileName);
            }

            oldProfileName = tempProfileName;
        }
        else if (reader->hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(reader->getErrorString());
            msg.setModal(true);
            msg.exec();
        }
        else if (reader->hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << reader->getErrorString() << endl;
        }
    }
    else
    {
        configBox->setItemText(0, tr("<New>"));
        configBox->setItemIcon(0, QIcon());

        removeCurrentButtons();

        QMetaObject::invokeMethod(&tabHelper, "reInitDevice", Qt::BlockingQueuedConnection);

        fillButtons();
        refreshSetButtons();
        refreshCopySetActions();
    }
}

void JoyTabWidget::saveAsConfig()
{
    int index = configBox->currentIndex();

    settings->getLock()->lock();

    int numberRecentProfiles = settings->value("NumberRecentProfiles", DEFAULTNUMBERPROFILES).toInt();
    QString filename;
    if (index == 0)
    {
        QString lookupDir = PadderCommon::preferredProfileDir(settings);
        settings->getLock()->unlock();

        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), lookupDir, tr("Config File (*.%1.amgp)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }
    else
    {
        settings->getLock()->unlock();

        QString configPath = configBox->itemData(index).toString();
        QFileInfo temp(configPath);
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), temp.absoluteDir().absolutePath(), tr("Config File (*.%1.amgp)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }

    if (!filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        QString deviceTypeName = joystick->getXmlName();
        QString fileSuffix = deviceTypeName.append(".amgp");
        if (fileinfo.suffix() != "xml" && fileinfo.suffix() != "amgp")
        {
            filename = filename.append(".").append(fileSuffix);
        }
        fileinfo.setFile(filename);

        /*XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);
        */
        QMetaObject::invokeMethod(&tabHelper, "writeConfigFile", Qt::BlockingQueuedConnection,
                                  Q_ARG(QString, fileinfo.absoluteFilePath()));
        XMLConfigWriter *writer = tabHelper.getWriter();

        if (writer->hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(writer->getErrorString());
            msg.setModal(true);
            msg.exec();
        }
        else if (writer->hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << writer->getErrorString() << endl;
        }
        else
        {
            int existingIndex = configBox->findData(fileinfo.absoluteFilePath());
            if (existingIndex == -1)
            {
                disconnectCheckUnsavedEvent();
                disconnectMainComboBoxEvents();

                if (numberRecentProfiles > 0 && configBox->count() == numberRecentProfiles+1)
                {
                    configBox->removeItem(numberRecentProfiles);
                }

                joystick->revertProfileEdited();
                QString tempProfileName = PadderCommon::getProfileName(fileinfo);
                if (!joystick->getProfileName().isEmpty())
                {
                    oldProfileName = joystick->getProfileName();
                    tempProfileName = oldProfileName;
                }

                configBox->insertItem(1, tempProfileName, fileinfo.absoluteFilePath());

                reconnectCheckUnsavedEvent();
                reconnectMainComboBoxEvents();

                configBox->setCurrentIndex(1);
                saveDeviceSettings(true);
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
            else
            {
                joystick->revertProfileEdited();
                if (!joystick->getProfileName().isEmpty())
                {
                    oldProfileName = joystick->getProfileName();
                }

                configBox->setItemIcon(existingIndex, QIcon());
                saveDeviceSettings(true);
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
        }
    }
}

void JoyTabWidget::changeJoyConfig(int index)
{
    disconnect(joystick, SIGNAL(profileUpdated()), this, SLOT(displayProfileEditNotification()));

    QString filename;
    if (index > 0)
    {
        filename = configBox->itemData(index).toString();
    }

    if (!filename.isEmpty())
    {
        removeCurrentButtons();
        emit forceTabUnflash(this);

        QMetaObject::invokeMethod(&tabHelper, "readConfigFile", Qt::BlockingQueuedConnection,
                                  Q_ARG(QString, filename));

        fillButtons();
        refreshSetButtons();
        refreshCopySetActions();
        configBox->setItemText(0, tr("<New>"));
        XMLConfigReader *reader = tabHelper.getReader();

        if (!reader->hasError())
        {
            QString profileName;
            if (!joystick->getProfileName().isEmpty())
            {
                profileName = joystick->getProfileName();
                oldProfileName = profileName;
            }
            else
            {
                QFileInfo profile(filename);
                oldProfileName = PadderCommon::getProfileName(profile);
                profileName = oldProfileName;
            }
            configBox->setItemText(index, profileName);
        }
        else if (reader->hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(reader->getErrorString());
            msg.setModal(true);
            msg.exec();
        }
        else if (reader->hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << reader->getErrorString() << endl;
        }
    }
    else if (index == 0)
    {
        removeCurrentButtons();
        emit forceTabUnflash(this);

        QMetaObject::invokeMethod(&tabHelper, "reInitDevice", Qt::BlockingQueuedConnection);

        fillButtons();
        refreshSetButtons();
        refreshCopySetActions();
        configBox->setItemText(0, tr("<New>"));
        oldProfileName = "";
    }

    comboBoxIndex = index;

    connect(joystick, SIGNAL(profileUpdated()), this, SLOT(displayProfileEditNotification()));
}

void JoyTabWidget::saveSettings()
{
    QString filename = "";
    QString lastfile = "";

    settings->getLock()->lock();

    int index = configBox->currentIndex();
    int currentjoy = 1;

    QString identifier = joystick->getStringIdentifier();

    QString controlEntryPrefix = QString("Controller%1").arg(identifier);
    QString controlEntryString = QString("Controller%1ConfigFile%2").arg(identifier);
    QString controlEntryLastSelected = QString("Controller%1LastSelected").arg(identifier);
    QString controlEntryProfileName = QString("Controller%1ProfileName%2").arg(joystick->getStringIdentifier());

    // Remove current settings for a controller
    QStringList tempkeys = settings->allKeys();
    QStringListIterator iter(tempkeys);
    while (iter.hasNext())
    {
        QString tempstring = iter.next();
        if (!identifier.isEmpty() && tempstring.startsWith(controlEntryPrefix))
        {
            settings->remove(tempstring);
        }
    }

    // Output currently selected profile as first profile on the list
    if (index != 0)
    {
        filename = lastfile = configBox->itemData(index).toString();
        QString profileText = configBox->itemText(index);
        if (!identifier.isEmpty())
        {
            QFileInfo profileBaseFile(filename);
            QString outputFilename = filename;

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
            if (profileBaseFile.isAbsolute())
            {
                QDir tempDir = profileBaseFile.dir();
                tempDir.cdUp();
                if (tempDir.path() == qApp->applicationDirPath())
                {
                    outputFilename = QString("%1/%2")
                            .arg(profileBaseFile.dir().dirName())
                            .arg(profileBaseFile.fileName());
                }
            }
#endif

            settings->setValue(controlEntryString.arg(currentjoy), outputFilename);

            if (PadderCommon::getProfileName(profileBaseFile) != profileText)
            {
                settings->setValue(controlEntryProfileName.arg(currentjoy), profileText);
            }
        }

        currentjoy++;
    }
    else
    {
        lastfile = "";
    }

    // Write the remaining profile locations to the settings file
    for (int i=1; i < configBox->count(); i++)
    {
        if (i != index)
        {
           filename = configBox->itemData(i).toString();
           QString profileText = configBox->itemText(i);
           if (!identifier.isEmpty())
           {
               QFileInfo profileBaseFile(filename);
               QString outputFilename = filename;

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
               if (profileBaseFile.isAbsolute())
               {
                   QDir tempDir = profileBaseFile.dir();
                   tempDir.cdUp();
                   if (tempDir.path() == qApp->applicationDirPath())
                   {
                       outputFilename = QString("%1/%2")
                               .arg(profileBaseFile.dir().dirName())
                               .arg(profileBaseFile.fileName());
                   }
               }
#endif
               settings->setValue(controlEntryString.arg(currentjoy), outputFilename);

               if (PadderCommon::getProfileName(profileBaseFile) != profileText)
               {
                   settings->setValue(controlEntryProfileName.arg(currentjoy), profileText);
               }
           }

           currentjoy++;
        }
    }

    if (!identifier.isEmpty())
    {
        QFileInfo profileBaseFile(lastfile);
        QString outputFilename = lastfile;

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
       if (profileBaseFile.isAbsolute())
       {
           QDir tempDir = profileBaseFile.dir();
           tempDir.cdUp();
           if (tempDir.path() == qApp->applicationDirPath())
           {
               outputFilename = QString("%1/%2")
                       .arg(profileBaseFile.dir().dirName())
                       .arg(profileBaseFile.fileName());
           }
       }
#endif

        settings->setValue(controlEntryLastSelected, outputFilename);
    }

    settings->getLock()->unlock();
}

void JoyTabWidget::loadSettings(bool forceRefresh)
{
    disconnect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));

    settings->getLock()->lock();

    if (configBox->count() > 1)
    {
        configBox->clear();
        configBox->addItem(tr("<New>"), "");
        configBox->setCurrentIndex(-1);
    }
    else if (forceRefresh)
    {
        configBox->setCurrentIndex(-1);
    }

    int shouldisplaynames = settings->value("DisplayNames", "0").toInt();
    if (shouldisplaynames == 1)
    {
        changeNameDisplay(shouldisplaynames);
    }

    int numberRecentProfiles = settings->value("NumberRecentProfiles", DEFAULTNUMBERPROFILES).toInt();
    bool autoOpenLastProfile = settings->value("AutoOpenLastProfile", true).toBool();

    settings->beginGroup("Controllers");

    QString controlEntryString = QString("Controller%1ConfigFile%2").arg(joystick->getStringIdentifier());
    QString controlEntryLastSelected = QString("Controller%1LastSelected").arg(joystick->getStringIdentifier());
    QString controlEntryProfileName = QString("Controller%1ProfileName%2").arg(joystick->getStringIdentifier());

    bool finished = false;
    for (int i=1; !finished; i++)
    {
        QString tempfilepath;

        if (!joystick->getStringIdentifier().isEmpty())
        {
            tempfilepath = settings->value(controlEntryString.arg(i), "").toString();
        }

        if (!tempfilepath.isEmpty())
        {
            QFileInfo fileInfo(tempfilepath);

            if (fileInfo.exists() && configBox->findData(fileInfo.absoluteFilePath()) == -1)
            {
                QString profileName = settings->value(controlEntryProfileName.arg(i), "").toString();
                profileName = !profileName.isEmpty() ? profileName : PadderCommon::getProfileName(fileInfo);
                configBox->addItem(profileName, fileInfo.absoluteFilePath());
            }
        }
        else
        {
            finished = true;
        }

        if (numberRecentProfiles > 0 && (i == numberRecentProfiles))
        {
            finished = true;
        }
    }

    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)), Qt::QueuedConnection);

    QString lastfile;

    if (!joystick->getStringIdentifier().isEmpty() && autoOpenLastProfile)
    {
        lastfile = settings->value(controlEntryLastSelected, "").toString();
    }

    settings->endGroup();
    settings->getLock()->unlock();

    if (!lastfile.isEmpty())
    {
        QString lastFileAbsolute = lastfile;

#if defined(Q_OS_WIN) && defined(WIN_PORTABLE_PACKAGE)
        QFileInfo lastFileInfo(lastfile);
        lastFileAbsolute = lastFileInfo.absoluteFilePath();
#endif

        int lastindex = configBox->findData(lastFileAbsolute);
        if (lastindex > 0)
        {
            configBox->setCurrentIndex(lastindex);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else if (configBox->currentIndex() != 0)
        {
            configBox->setCurrentIndex(0);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
    }
    else if (configBox->currentIndex() != 0)
    {
        configBox->setCurrentIndex(0);
        emit joystickConfigChanged(joystick->getJoyNumber());
    }
}

QHash<int, QString>* JoyTabWidget::recentConfigs()
{
    QHash<int, QString> *temp = new QHash<int, QString>();
    for (int i=1; i < configBox->count(); i++)
    {
        QString current = configBox->itemText(i);
        temp->insert(i, current);
    }

    return temp;
}

void JoyTabWidget::setCurrentConfig(int index)
{
    // Allow 0 to select new/'null' config and therefore disable any mapping
    if (index >= 0 && index < configBox->count())
    {
        configBox->setCurrentIndex(index);
    }
}

int JoyTabWidget::getCurrentConfigIndex()
{
    return configBox->currentIndex();
}

QString JoyTabWidget::getCurrentConfigName()
{
    return configBox->currentText();
}

QString JoyTabWidget::getConfigName(int index)
{
    return configBox->itemText(index);
}

void JoyTabWidget::changeCurrentSet(int index)
{
    int currentPage = stackedWidget_2->currentIndex();
    QPushButton *oldSetButton = 0;
    QPushButton *activeSetButton = 0;

    switch (currentPage)
    {
        case 0: oldSetButton = setPushButton1; break;
        case 1: oldSetButton = setPushButton2; break;
        case 2: oldSetButton = setPushButton3; break;
        case 3: oldSetButton = setPushButton4; break;
        case 4: oldSetButton = setPushButton5; break;
        case 5: oldSetButton = setPushButton6; break;
        case 6: oldSetButton = setPushButton7; break;
        case 7: oldSetButton = setPushButton8; break;
        default: break;
    }

    if (oldSetButton)
    {
        oldSetButton->setProperty("setActive", false);
        oldSetButton->style()->unpolish(oldSetButton);
        oldSetButton->style()->polish(oldSetButton);
    }

    joystick->setActiveSetNumber(index);
    stackedWidget_2->setCurrentIndex(index);

    switch (index)
    {
        case 0: activeSetButton = setPushButton1; break;
        case 1: activeSetButton = setPushButton2; break;
        case 2: activeSetButton = setPushButton3; break;
        case 3: activeSetButton = setPushButton4; break;
        case 4: activeSetButton = setPushButton5; break;
        case 5: activeSetButton = setPushButton6; break;
        case 6: activeSetButton = setPushButton7; break;
        case 7: activeSetButton = setPushButton8; break;
        default: break;
    }

    if (activeSetButton)
    {
        activeSetButton->setProperty("setActive", true);
        activeSetButton->style()->unpolish(activeSetButton);
        activeSetButton->style()->polish(activeSetButton);
    }
}

void JoyTabWidget::changeSetOne()
{
    changeCurrentSet(0);
}

void JoyTabWidget::changeSetTwo()
{
    changeCurrentSet(1);
}

void JoyTabWidget::changeSetThree()
{
    changeCurrentSet(2);
}

void JoyTabWidget::changeSetFour()
{
    changeCurrentSet(3);
}

void JoyTabWidget::changeSetFive()
{
    changeCurrentSet(4);
}

void JoyTabWidget::changeSetSix()
{
    changeCurrentSet(5);
}

void JoyTabWidget::changeSetSeven()
{
    changeCurrentSet(6);
}

void JoyTabWidget::changeSetEight()
{
    changeCurrentSet(7);
}

void JoyTabWidget::showStickAssignmentDialog()
{
    Joystick *temp = static_cast<Joystick*>(joystick);
    AdvanceStickAssignmentDialog *dialog = new AdvanceStickAssignmentDialog(temp, this);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshButtons()));
    dialog->show();
}

void JoyTabWidget::loadConfigFile(QString fileLocation)
{
    checkForUnsavedProfile(-1);

    if (!joystick->isDeviceEdited())
    {
        int numberRecentProfiles = settings->value("NumberRecentProfiles", DEFAULTNUMBERPROFILES).toInt();
        QFileInfo fileinfo(fileLocation);
        if (fileinfo.exists() && (fileinfo.suffix() == "xml" || fileinfo.suffix() == "amgp"))
        {
            int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
            if (searchIndex == -1)
            {
                disconnectCheckUnsavedEvent();
                disconnectMainComboBoxEvents();

                if (numberRecentProfiles > 0 && configBox->count() == numberRecentProfiles+1)
                {
                    configBox->removeItem(numberRecentProfiles-1);
                    //configBox->removeItem(5);
                }
                configBox->insertItem(1, PadderCommon::getProfileName(fileinfo), fileinfo.absoluteFilePath());

                reconnectCheckUnsavedEvent();
                reconnectMainComboBoxEvents();

                configBox->setCurrentIndex(1);
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
            else if (searchIndex != configBox->currentIndex())
            {
                configBox->setCurrentIndex(searchIndex);
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
        }
    }
}

void JoyTabWidget::showQuickSetDialog()
{
    QuickSetDialog *dialog = new QuickSetDialog(joystick, this);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshButtons()));
    dialog->show();
}

void JoyTabWidget::showKeyDelayDialog()
{
    ExtraProfileSettingsDialog *dialog = new ExtraProfileSettingsDialog(joystick, this);
    dialog->show();
}

void JoyTabWidget::showSetNamesDialog()
{
    SetNamesDialog *dialog = new SetNamesDialog(joystick, this);
    connect(dialog, SIGNAL(accepted()), this, SLOT(refreshSetButtons()));
    connect(dialog, SIGNAL(accepted()), this, SLOT(refreshCopySetActions()));
    dialog->show();
}

void JoyTabWidget::removeCurrentButtons()
{
    joystick->disconnectPropertyUpdatedConnection();
    disconnect(joystick, SIGNAL(setChangeActivated(int)), this, SLOT(changeCurrentSet(int)));

    for (int i=0; i < Joystick::NUMBER_JOYSETS; i++)
    {
        SetJoystick *currentSet = joystick->getSetJoystick(i);
        removeSetButtons(currentSet);
    }
}

InputDevice *JoyTabWidget::getJoystick()
{
    return joystick;
}

void JoyTabWidget::removeConfig()
{
    int currentIndex = configBox->currentIndex();
    if (currentIndex > 0)
    {
        configBox->removeItem(currentIndex);
        saveDeviceSettings(true);
        emit joystickConfigChanged(joystick->getJoyNumber());
    }
}

void JoyTabWidget::toggleNames()
{
    displayingNames = !displayingNames;
    namesPushButton->setProperty("isDisplayingNames", displayingNames);
    namesPushButton->style()->unpolish(namesPushButton);
    namesPushButton->style()->polish(namesPushButton);
    emit namesDisplayChanged(displayingNames);
}

void JoyTabWidget::unloadConfig()
{
    configBox->setCurrentIndex(0);
}

void JoyTabWidget::saveDeviceSettings(bool sync)
{
    settings->getLock()->lock();
    settings->beginGroup("Controllers");
    settings->getLock()->unlock();

    saveSettings();

    settings->getLock()->lock();
    settings->endGroup();
    if (sync)
    {
        settings->sync();
    }
    settings->getLock()->unlock();
}

void JoyTabWidget::loadDeviceSettings()
{
    //settings.beginGroup("Controllers");
    loadSettings();
    //settings.endGroup();
}

bool JoyTabWidget::isDisplayingNames()
{
    return displayingNames;
}

void JoyTabWidget::changeNameDisplay(bool displayNames)
{
    displayingNames = displayNames;
    namesPushButton->setProperty("isDisplayingNames", displayingNames);
    namesPushButton->style()->unpolish(namesPushButton);
    namesPushButton->style()->polish(namesPushButton);
}

void JoyTabWidget::refreshSetButtons()
{
    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        QPushButton *tempSetButton = 0;
        QAction *tempSetAction = 0;
        SetJoystick *tempSet = joystick->getSetJoystick(i);
        switch (i)
        {
            case 0:
                tempSetButton = setPushButton1;
                tempSetAction = setAction1;
                break;
            case 1:
                tempSetButton = setPushButton2;
                tempSetAction = setAction2;
                break;
            case 2:
                tempSetButton = setPushButton3;
                tempSetAction = setAction3;
                break;
            case 3:
                tempSetButton = setPushButton4;
                tempSetAction = setAction4;
                break;
            case 4:
                tempSetButton = setPushButton5;
                tempSetAction = setAction5;
                break;
            case 5:
                tempSetButton = setPushButton6;
                tempSetAction = setAction6;
                break;
            case 6:
                tempSetButton = setPushButton7;
                tempSetAction = setAction7;
                break;
            case 7:
                tempSetButton = setPushButton8;
                tempSetAction = setAction8;
                break;
        }

        if (!tempSet->getName().isEmpty())
        {
            QString tempName = tempSet->getName();
            QString tempNameEscaped = tempName;
            tempNameEscaped.replace("&", "&&");
            tempSetButton->setText(tempNameEscaped);
            tempSetButton->setToolTip(tempName);

            tempSetAction->setText(tr("Set").append(" %1: %2").arg(i+1).arg(tempNameEscaped));
        }
        else
        {
            tempSetButton->setText(QString::number(i+1));
            tempSetButton->setToolTip("");

            tempSetAction->setText(tr("Set").append(" %1").arg(i+1));
        }
    }
}

void JoyTabWidget::displayProfileEditNotification()
{
    int currentIndex = configBox->currentIndex();
    configBox->setItemIcon(currentIndex, QIcon::fromTheme("document-save-as",
                                         QIcon(":/icons/16x16/actions/document-save.png")));
}

void JoyTabWidget::removeProfileEditNotification()
{
    for (int i=0; i < configBox->count(); i++)
    {
        if (!configBox->itemIcon(i).isNull())
        {
            configBox->setItemIcon(i, QIcon());
        }
    }
}

void JoyTabWidget::retranslateUi()
{
    removeButton->setText(tr("Remove"));
    removeButton->setToolTip(tr("Remove configuration from recent list."));

    loadButton->setText(tr("Load"));
    loadButton->setToolTip(tr("Load configuration file."));

    saveButton->setText(tr("Save"));
    saveButton->setToolTip(tr("Save changes to configuration file."));

    saveAsButton->setText(tr("Save As"));
    saveAsButton->setToolTip(tr("Save changes to a new configuration file."));

    setsMenuButton->setText(tr("Sets"));
    setAction1->setText(tr("Set 1"));
    setAction2->setText(tr("Set 2"));
    setAction3->setText(tr("Set 3"));
    setAction4->setText(tr("Set 4"));
    setAction5->setText(tr("Set 5"));
    setAction6->setText(tr("Set 6"));
    setAction7->setText(tr("Set 7"));
    setAction8->setText(tr("Set 8"));

    refreshSetButtons();
    refreshCopySetActions();

    gameControllerMappingPushButton->setText(tr("Controller Mapping"));
    stickAssignPushButton->setText(tr("Stick/Pad Assign"));
    quickSetPushButton->setText(tr("Quick Set"));
    resetButton->setText(tr("Reset"));

    namesPushButton->setText(tr("Names"));
    namesPushButton->setToolTip(tr("Toggle button name displaying."));

    delayButton->setText(tr("Pref"));
    delayButton->setToolTip(tr("Change global profile settings."));

    resetButton->setText(tr("Reset"));
    resetButton->setToolTip(tr("Revert changes to the configuration. Reload configuration file."));
    refreshButtons();
}

void JoyTabWidget::checkForUnsavedProfile(int newindex)
{
    if (joystick->isDeviceEdited())
    {
        disconnectCheckUnsavedEvent();
        disconnectMainComboBoxEvents();

        if (configBox->currentIndex() != comboBoxIndex)
        {
            configBox->setCurrentIndex(comboBoxIndex);
        }

        QMessageBox msg;
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msg.setWindowTitle(tr("Save Profile Changes?"));
        if (comboBoxIndex == 0)
        {
            msg.setText(tr("Changes to the new profile have not been saved. Would you like to save or discard the current profile?"));
        }
        else
        {
            msg.setText(tr("Changes to the profile \"%1\" have not been saved. Would you like to save or discard changes to the current profile?")
                        .arg(configBox->currentText()));
        }

        int status = msg.exec();
        if (status == QMessageBox::Save)
        {
            saveConfigFile();
            reconnectCheckUnsavedEvent();
            reconnectMainComboBoxEvents();
            if (newindex > -1)
            {
                configBox->setCurrentIndex(newindex);
            }

        }
        else if (status == QMessageBox::Discard)
        {
            joystick->revertProfileEdited();
            configBox->setItemText(comboBoxIndex, oldProfileName);
            reconnectCheckUnsavedEvent();
            reconnectMainComboBoxEvents();
            if (newindex > -1)
            {
                configBox->setCurrentIndex(newindex);
            }

        }
        else if (status == QMessageBox::Cancel)
        {
            reconnectCheckUnsavedEvent();
            reconnectMainComboBoxEvents();
        }
    }
}

bool JoyTabWidget::discardUnsavedProfileChanges()
{
    bool discarded = true;

    if (joystick->isDeviceEdited())
    {
        disconnectCheckUnsavedEvent();

        QMessageBox msg;
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msg.setWindowTitle(tr("Save Profile Changes?"));
        int currentIndex = configBox->currentIndex();
        if (currentIndex == 0)
        {
            msg.setText(tr("Changes to the new profile have not been saved. Would you like to save or discard the current profile?"));
        }
        else
        {
            msg.setText(tr("Changes to the profile \"%1\" have not been saved. Would you like to save or discard changes to the current profile?")
                        .arg(configBox->currentText()));
        }

        int status = msg.exec();
        if (status == QMessageBox::Save)
        {
            saveConfigFile();
            if (currentIndex == 0 && currentIndex == configBox->currentIndex())
            {
                discarded = false;
            }
        }
        else if (status == QMessageBox::Discard)
        {
            joystick->revertProfileEdited();
            configBox->setItemText(currentIndex, oldProfileName);
            resetJoystick();
        }
        else if (status == QMessageBox::Cancel)
        {
            discarded = false;
        }

        disconnectMainComboBoxEvents();
        reconnectCheckUnsavedEvent();
        reconnectMainComboBoxEvents();
    }

    return discarded;
}

void JoyTabWidget::disconnectMainComboBoxEvents()
{
    disconnect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));
    disconnect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(removeProfileEditNotification()));
    disconnect(joystick, SIGNAL(profileNameEdited(QString)), this, SLOT(editCurrentProfileItemText(QString)));
}

void JoyTabWidget::reconnectMainComboBoxEvents()
{
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)), Qt::QueuedConnection);
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(removeProfileEditNotification()), Qt::QueuedConnection);
    connect(joystick, SIGNAL(profileNameEdited(QString)), this, SLOT(editCurrentProfileItemText(QString)));
}

void JoyTabWidget::disconnectCheckUnsavedEvent()
{
    disconnect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUnsavedProfile(int)));
}

void JoyTabWidget::reconnectCheckUnsavedEvent()
{
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForUnsavedProfile(int)));
}

void JoyTabWidget::refreshButtons()
{
    removeCurrentButtons();
    fillButtons();
}

void JoyTabWidget::checkStickDisplay()
{
    JoyControlStickButton *button = static_cast<JoyControlStickButton*>(sender());
    JoyControlStick *stick = button->getStick();
    if (stick && stick->hasSlotsAssigned())
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkDPadButtonDisplay()
{
    JoyDPadButton *button = static_cast<JoyDPadButton*>(sender());
    JoyDPad *dpad = button->getDPad();
    if (dpad && dpad->hasSlotsAssigned())
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkAxisButtonDisplay()
{
    JoyAxisButton *button = static_cast<JoyAxisButton*>(sender());
    if (button->getAssignedSlots()->count() > 0)
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkButtonDisplay()
{
    JoyButton *button = static_cast<JoyButton*>(sender());
    if (button->getAssignedSlots()->count() > 0)
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkStickEmptyDisplay()
{
    StickPushButtonGroup *group = static_cast<StickPushButtonGroup*>(sender());
    JoyControlStick *stick = group->getStick();
    //JoyControlStickButton *button = static_cast<JoyControlStickButton*>(sender());
    //JoyControlStick *stick = button->getStick();
    if (stick && !stick->hasSlotsAssigned())
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkDPadButtonEmptyDisplay()
{
    DPadPushButtonGroup *group = static_cast<DPadPushButtonGroup*>(sender());
    JoyDPad *dpad = group->getDPad();
    //JoyDPadButton *button = static_cast<JoyDPadButton*>(sender());
    //JoyDPad *dpad = button->getDPad();
    if (dpad && !dpad->hasSlotsAssigned())
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkAxisButtonEmptyDisplay()
{
    JoyAxisButton *button = static_cast<JoyAxisButton*>(sender());
    if (button->getAssignedSlots()->count() == 0)
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkButtonEmptyDisplay()
{
    JoyButton *button = static_cast<JoyButton*>(sender());
    if (button->getAssignedSlots()->count() == 0)
    {
        SetJoystick *currentSet = joystick->getActiveSetJoystick();
        removeSetButtons(currentSet);
        fillSetButtons(currentSet);
    }
}

void JoyTabWidget::checkHideEmptyOption()
{
    bool currentHideEmptyButtons = settings->value("HideEmptyButtons", false).toBool();
    if (currentHideEmptyButtons != hideEmptyButtons)
    {
        hideEmptyButtons = currentHideEmptyButtons;
        refreshButtons();
    }
}

void JoyTabWidget::fillSetButtons(SetJoystick *set)
{
    int row = 0;
    int column = 0;

    //QWidget *child = 0;
    QGridLayout *current_layout = 0;
    switch (set->getIndex())
    {
        case 0:
        {
            current_layout = gridLayout;
            break;
        }
        case 1:
        {
            current_layout = gridLayout2;
            break;
        }
        case 2:
        {
            current_layout = gridLayout3;
            break;
        }
        case 3:
        {
            current_layout = gridLayout4;
            break;
        }
        case 4:
        {
            current_layout = gridLayout5;
            break;
        }
        case 5:
        {
            current_layout = gridLayout6;
            break;
        }
        case 6:
        {
            current_layout = gridLayout7;
            break;
        }
        case 7:
        {
            current_layout = gridLayout8;
            break;
        }
        default:
            break;
    }

    /*while (current_layout && current_layout->count() > 0)
    {
        child = current_layout->takeAt(0)->widget();
        current_layout->removeWidget (child);
        delete child;
        child = 0;
    }
    */

    SetJoystick *currentSet = set;
    currentSet->establishPropertyUpdatedConnection();

    QGridLayout *stickGrid = 0;
    QGroupBox *stickGroup = 0;
    int stickGridColumn = 0;
    int stickGridRow = 0;

    for (int j=0; j < joystick->getNumberSticks(); j++)
    {
        JoyControlStick *stick = currentSet->getJoyStick(j);
        stick->establishPropertyUpdatedConnection();
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();

        if (!hideEmptyButtons || stick->hasSlotsAssigned())
        {
            if (!stickGroup)
            {
                stickGroup = new QGroupBox(tr("Sticks"), this);
            }

            if (!stickGrid)
            {
                stickGrid = new QGridLayout();
                stickGridColumn = 0;
                stickGridRow = 0;
            }

            QWidget *groupContainer = new QWidget(stickGroup);
            StickPushButtonGroup *stickButtonGroup = new StickPushButtonGroup(stick, displayingNames, groupContainer);
            if (hideEmptyButtons)
            {
                connect(stickButtonGroup, SIGNAL(buttonSlotChanged()), this, SLOT(checkStickEmptyDisplay()));
            }

            connect(namesPushButton, SIGNAL(clicked()), stickButtonGroup, SLOT(toggleNameDisplay()));

            if (stickGridColumn > 1)
            {
                stickGridColumn = 0;
                stickGridRow++;
            }

            groupContainer->setLayout(stickButtonGroup);
            stickGrid->addWidget(groupContainer, stickGridRow, stickGridColumn);
            stickGridColumn++;
        }
        else
        {
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> tempiter(*stickButtons);
            while (tempiter.hasNext())
            {
                JoyControlStickButton *button = tempiter.next().value();
                button->establishPropertyUpdatedConnections();
                connect(button, SIGNAL(slotsChanged()), this, SLOT(checkStickDisplay()));
            }
        }
    }

    if (stickGroup)
    {
        QSpacerItem *tempspacer = new QSpacerItem(10, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);
        QVBoxLayout *tempvbox = new QVBoxLayout;
        tempvbox->addLayout(stickGrid);
        tempvbox->addItem(tempspacer);
        stickGroup->setLayout(tempvbox);
        current_layout->addWidget(stickGroup, row, column, 1, 2, Qt::AlignTop);
        row++;
    }

    column = 0;

    QGridLayout *hatGrid = 0;
    QGroupBox *hatGroup = 0;
    int hatGridColumn = 0;
    int hatGridRow = 0;
    for (int j=0; j < joystick->getNumberHats(); j++)
    {
        JoyDPad *dpad = currentSet->getJoyDPad(j);
        dpad->establishPropertyUpdatedConnection();
        QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

        if (!hideEmptyButtons || dpad->hasSlotsAssigned())
        {
            if (!hatGroup)
            {
                hatGroup = new QGroupBox(tr("DPads"), this);
            }

            if (!hatGrid)
            {
                hatGrid = new QGridLayout();
                hatGridColumn = 0;
                hatGridRow = 0;
            }

            QWidget *groupContainer = new QWidget(hatGroup);
            DPadPushButtonGroup *dpadButtonGroup = new DPadPushButtonGroup(dpad, displayingNames, groupContainer);
            if (hideEmptyButtons)
            {
                connect(dpadButtonGroup, SIGNAL(buttonSlotChanged()), this, SLOT(checkDPadButtonEmptyDisplay()));
            }

            connect(namesPushButton, SIGNAL(clicked()), dpadButtonGroup, SLOT(toggleNameDisplay()));

            if (hatGridColumn > 1)
            {
                hatGridColumn = 0;
                hatGridRow++;
            }

            groupContainer->setLayout(dpadButtonGroup);
            hatGrid->addWidget(groupContainer, hatGridRow, hatGridColumn);
            hatGridColumn++;
        }
        else
        {
            QHashIterator<int, JoyDPadButton*> tempiter(*buttons);
            while (tempiter.hasNext())
            {
                JoyDPadButton *button = tempiter.next().value();
                button->establishPropertyUpdatedConnections();
                connect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonDisplay()));
            }
        }
    }

    for (int j=0; j < joystick->getNumberVDPads(); j++)
    {
        VDPad *vdpad = currentSet->getVDPad(j);
        vdpad->establishPropertyUpdatedConnection();
        QHash<int, JoyDPadButton*> *buttons = vdpad->getButtons();

        if (!hideEmptyButtons || vdpad->hasSlotsAssigned())
        {
            if (!hatGroup)
            {
                hatGroup = new QGroupBox(tr("DPads"), this);
            }

            if (!hatGrid)
            {
                hatGrid = new QGridLayout();
                hatGridColumn = 0;
                hatGridRow = 0;
            }

            QWidget *groupContainer = new QWidget(hatGroup);
            DPadPushButtonGroup *dpadButtonGroup = new DPadPushButtonGroup(vdpad, displayingNames, groupContainer);
            if (hideEmptyButtons)
            {
                connect(dpadButtonGroup, SIGNAL(buttonSlotChanged()), this, SLOT(checkDPadButtonEmptyDisplay()));
            }

            connect(namesPushButton, SIGNAL(clicked()), dpadButtonGroup, SLOT(toggleNameDisplay()));

            if (hatGridColumn > 1)
            {
                hatGridColumn = 0;
                hatGridRow++;
            }

            groupContainer->setLayout(dpadButtonGroup);
            hatGrid->addWidget(groupContainer, hatGridRow, hatGridColumn);
            hatGridColumn++;

        }
        else
        {
            QHashIterator<int, JoyDPadButton*> tempiter(*buttons);
            while (tempiter.hasNext())
            {
                JoyDPadButton *button = tempiter.next().value();
                button->establishPropertyUpdatedConnections();
                connect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonDisplay()));
            }
        }
    }

    if (hatGroup)
    {
        QSpacerItem *tempspacer = new QSpacerItem(10, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);
        QVBoxLayout *tempvbox = new QVBoxLayout;
        tempvbox->addLayout(hatGrid);
        tempvbox->addItem(tempspacer);
        hatGroup->setLayout(tempvbox);
        current_layout->addWidget(hatGroup, row, column, 1, 2, Qt::AlignTop);
        row++;
    }

    column = 0;

    for (int j=0; j < joystick->getNumberAxes(); j++)
    {
        JoyAxis *axis = currentSet->getJoyAxis(j);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            JoyAxisButton *naxisbutton = axis->getNAxisButton();

            if (!hideEmptyButtons ||
               (paxisbutton->getAssignedSlots()->count() > 0 ||
                naxisbutton->getAssignedSlots()->count() > 0))
            {
                JoyAxisWidget *axisWidget = new JoyAxisWidget(axis, displayingNames, this);
                axisWidget->setText(axis->getName());
                axisWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                axisWidget->setMinimumSize(200, 24);

                connect(axisWidget, SIGNAL(clicked()), this, SLOT(showAxisDialog()));
                connect(namesPushButton, SIGNAL(clicked()), axisWidget, SLOT(toggleNameDisplay()));
                if (hideEmptyButtons)
                {
                    connect(paxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonEmptyDisplay()));
                    connect(naxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonEmptyDisplay()));
                }

                if (column > 1)
                {
                    column = 0;
                    row++;
                }
                current_layout->addWidget(axisWidget, row, column);
                column++;
            }
            else
            {
                paxisbutton->establishPropertyUpdatedConnections();
                naxisbutton->establishPropertyUpdatedConnections();

                connect(paxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonDisplay()));
                connect(naxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonDisplay()));
            }
        }
    }

    for (int j=0; j < joystick->getNumberButtons(); j++)
    {
        JoyButton *button = currentSet->getJoyButton(j);
        if (button && !button->isPartVDPad())
        {
            button->establishPropertyUpdatedConnections();

            if (!hideEmptyButtons || button->getAssignedSlots()->count() > 0)
            {
                JoyButtonWidget *buttonWidget = new JoyButtonWidget (button, displayingNames, this);
                buttonWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                buttonWidget->setText(buttonWidget->text());
                buttonWidget->setMinimumSize(200, 24);

                connect(buttonWidget, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), buttonWidget, SLOT(toggleNameDisplay()));
                if (hideEmptyButtons)
                {
                    connect(button, SIGNAL(slotsChanged()), this, SLOT(checkButtonEmptyDisplay()));
                }

                if (column > 1)
                {
                    column = 0;
                    row++;
                }

                current_layout->addWidget(buttonWidget, row, column);
                column++;
            }
            else
            {
                button->establishPropertyUpdatedConnections();
                connect(button, SIGNAL(slotsChanged()), this, SLOT(checkButtonDisplay()));
            }
        }
    }

    if (current_layout->count() == 0)
    {
        QLabel *newlabel = new QLabel(tr("No buttons have been assigned. Please use Quick Set to assign keys\nto buttons or disable hiding empty buttons."));
        current_layout->addWidget(newlabel, 0, 0, Qt::AlignCenter);
    }
}

void JoyTabWidget::removeSetButtons(SetJoystick *set)
{
    SetJoystick *currentSet = set;
    currentSet->disconnectPropertyUpdatedConnection();

    QLayoutItem *child = 0;
    QGridLayout *current_layout = 0;
    switch (currentSet->getIndex())
    {
        case 0:
        {
            current_layout = gridLayout;
            break;
        }
        case 1:
        {
            current_layout = gridLayout2;
            break;
        }
        case 2:
        {
            current_layout = gridLayout3;
            break;
        }
        case 3:
        {
            current_layout = gridLayout4;
            break;
        }
        case 4:
        {
            current_layout = gridLayout5;
            break;
        }
        case 5:
        {
            current_layout = gridLayout6;
            break;
        }
        case 6:
        {
            current_layout = gridLayout7;
            break;
        }
        case 7:
        {
            current_layout = gridLayout8;
            break;
        }
    }

    while (current_layout && (child = current_layout->takeAt(0)) != 0)
    {
        current_layout->removeWidget(child->widget());
        delete child->widget();
        delete child;
        child = 0;
    }

    for (int j=0; j < joystick->getNumberSticks(); j++)
    {
        JoyControlStick *stick = currentSet->getJoyStick(j);
        stick->disconnectPropertyUpdatedConnection();
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();

        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> tempiter(*stickButtons);
        while (tempiter.hasNext())
        {
            JoyControlStickButton *button = tempiter.next().value();
            button->disconnectPropertyUpdatedConnections();
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkStickDisplay()));
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkStickEmptyDisplay()));
        }
    }

    for (int j=0; j < joystick->getNumberHats(); j++)
    {
        JoyDPad *dpad = currentSet->getJoyDPad(j);
        dpad->establishPropertyUpdatedConnection();
        QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

        QHashIterator<int, JoyDPadButton*> tempiter(*buttons);
        while (tempiter.hasNext())
        {
            JoyDPadButton *button = tempiter.next().value();
            button->disconnectPropertyUpdatedConnections();
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonDisplay()));
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonEmptyDisplay()));
        }
    }

    for (int j=0; j < joystick->getNumberVDPads(); j++)
    {
        VDPad *vdpad = currentSet->getVDPad(j);
        vdpad->establishPropertyUpdatedConnection();
        QHash<int, JoyDPadButton*> *buttons = vdpad->getButtons();

        QHashIterator<int, JoyDPadButton*> tempiter(*buttons);
        while (tempiter.hasNext())
        {
            JoyDPadButton *button = tempiter.next().value();
            button->disconnectPropertyUpdatedConnections();
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonDisplay()));
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkDPadButtonEmptyDisplay()));
        }
    }

    for (int j=0; j < joystick->getNumberAxes(); j++)
    {
        JoyAxis *axis = currentSet->getJoyAxis(j);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            JoyAxisButton *naxisbutton = axis->getNAxisButton();

            paxisbutton->disconnectPropertyUpdatedConnections();
            naxisbutton->disconnectPropertyUpdatedConnections();

            disconnect(paxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonDisplay()));
            disconnect(naxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonDisplay()));
            disconnect(paxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonEmptyDisplay()));
            disconnect(naxisbutton, SIGNAL(slotsChanged()), this, SLOT(checkAxisButtonEmptyDisplay()));
        }
    }

    for (int j=0; j < joystick->getNumberButtons(); j++)
    {
        JoyButton *button = currentSet->getJoyButton(j);
        if (button && !button->isPartVDPad())
        {
            button->disconnectPropertyUpdatedConnections();
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkButtonDisplay()));
            disconnect(button, SIGNAL(slotsChanged()), this, SLOT(checkButtonEmptyDisplay()));
        }
    }
}

void JoyTabWidget::editCurrentProfileItemText(QString text)
{
    int currentIndex = configBox->currentIndex();
    if (currentIndex >= 0)
    {
        if (!text.isEmpty())
        {
            configBox->setItemText(currentIndex, text);
        }
        else if (currentIndex == 0)
        {
            configBox->setItemText(currentIndex, tr("<New>"));
        }
        else if (currentIndex > 0)
        {
            QFileInfo profileName(configBox->itemData(currentIndex).toString());
            configBox->setItemText(currentIndex, PadderCommon::getProfileName(profileName));
        }
    }
}

#ifdef Q_OS_WIN
void JoyTabWidget::deviceKeyRepeatSettings()
{
    bool keyRepeatActive = settings->value("KeyRepeat/KeyRepeatEnabled", true).toBool();
    int keyRepeatDelay = settings->value("KeyRepeat/KeyRepeatDelay", InputDevice::DEFAULTKEYREPEATDELAY).toInt();
    int keyRepeatRate = settings->value("KeyRepeat/KeyRepeatRate", InputDevice::DEFAULTKEYREPEATRATE).toInt();

    joystick->setKeyRepeatStatus(keyRepeatActive);
    joystick->setKeyRepeatDelay(keyRepeatDelay);
    joystick->setKeyRepeatRate(keyRepeatRate);
}
#endif

void JoyTabWidget::refreshCopySetActions()
{
    copySetMenu->clear();

    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick *tempSet = joystick->getSetJoystick(i);
        QAction *newaction = 0;
        if (!tempSet->getName().isEmpty())
        {
            QString tempName = tempSet->getName();
            QString tempNameEscaped = tempName;
            tempNameEscaped.replace("&", "&&");
            newaction = new QAction(tr("Set %1: %2").arg(i+1).arg(tempNameEscaped), copySetMenu);
        }
        else
        {
            newaction = new QAction(tr("Set %1").arg(i+1), copySetMenu);
        }

        newaction->setData(i);
        connect(newaction, SIGNAL(triggered()), this, SLOT(performSetCopy()));
        copySetMenu->addAction(newaction);
    }

    connect(copySetMenu, SIGNAL(aboutToShow()), this, SLOT(disableCopyCurrentSet()));
}

void JoyTabWidget::performSetCopy()
{
    QAction *action = static_cast<QAction*>(sender());
    int sourceSetIndex = action->data().toInt();
    SetJoystick *sourceSet = joystick->getSetJoystick(sourceSetIndex);
    QString sourceName;
    if (!sourceSet->getName().isEmpty())
    {
        QString tempNameEscaped = sourceSet->getName();
        tempNameEscaped.replace("&", "&&");
        sourceName = tr("Set %1: %2").arg(sourceSetIndex+1).arg(tempNameEscaped);
    }
    else
    {
        sourceName = tr("Set %1").arg(sourceSetIndex+1);
    }

    SetJoystick *destSet = joystick->getActiveSetJoystick();
    if (sourceSet && destSet)
    {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setWindowTitle(tr("Copy Set Assignments"));
        msgBox.setText(tr("Are you sure you want to copy the assignments and device properties from %1?").arg(sourceName));
        int status = msgBox.exec();
        if (status == QMessageBox::Yes)
        {
            PadderCommon::lockInputDevices();

            removeSetButtons(destSet);

            QMetaObject::invokeMethod(sourceSet, "copyAssignments", Qt::BlockingQueuedConnection,
                                      Q_ARG(SetJoystick*, destSet));

            //sourceSet->copyAssignments(destSet);
            fillSetButtons(destSet);

            PadderCommon::unlockInputDevices();
        }
    }
}

void JoyTabWidget::disableCopyCurrentSet()
{
    SetJoystick *activeSet = joystick->getActiveSetJoystick();
    QMenu *menu = static_cast<QMenu*>(sender());
    QList<QAction*> actions = menu->actions();
    QListIterator<QAction*> iter(actions);
    while (iter.hasNext())
    {
        QAction *action = iter.next();
        if (action->data().toInt() == activeSet->getIndex())
        {
            action->setEnabled(false);
        }
        else
        {
            action->setEnabled(true);
        }
    }
}

#ifdef USE_SDL_2
void JoyTabWidget::openGameControllerMappingWindow()
{
    GameControllerMappingDialog *dialog = new GameControllerMappingDialog(joystick, settings, this);
    dialog->show();
    connect(dialog, SIGNAL(mappingUpdate(QString,InputDevice*)), this, SLOT(propogateMappingUpdate(QString, InputDevice*)));
}

void JoyTabWidget::propogateMappingUpdate(QString mapping, InputDevice *device)
{
    emit mappingUpdated(mapping, device);
}

#endif

void JoyTabWidget::refreshHelperThread()
{
    tabHelper.moveToThread(joystick->thread());
}

void JoyTabWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        retranslateUi();
    }

    QWidget::changeEvent(event);
}

