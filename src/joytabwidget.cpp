#include <QDebug>
#include <QLayoutItem>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextStream>
#include <QStringListIterator>


#include "joytabwidget.h"
#include "joyaxiswidget.h"
#include "joybuttonwidget.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "buttoneditdialog.h"
#include "joycontrolstickeditdialog.h"
#include "joycontrolstickpushbutton.h"
#include "advancestickassignmentdialog.h"
#include "joycontrolstickbuttonpushbutton.h"
#include "dpadpushbutton.h"
#include "dpadeditdialog.h"
#include "joydpadbuttonwidget.h"
#include "quicksetdialog.h"

#ifdef USE_SDL_2
#include "gamecontroller/gamecontroller.h"
#endif

JoyTabWidget::JoyTabWidget(InputDevice *joystick, QWidget *parent) :
    QWidget(parent)
{
    this->joystick = joystick;

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
    QScrollArea *scrollArea = new QScrollArea(page);
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
    QScrollArea *scrollArea2 = new QScrollArea(page_2);
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
    QScrollArea *scrollArea3 = new QScrollArea(page_3);
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
    QScrollArea *scrollArea4 = new QScrollArea(page_4);
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
    QScrollArea *scrollArea5 = new QScrollArea(page_5);
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
    QScrollArea *scrollArea6 = new QScrollArea(page_6);
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
    QScrollArea *scrollArea7 = new QScrollArea(page_7);
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
    QScrollArea *scrollArea8 = new QScrollArea(page_8);
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
    if (qobject_cast<GameController*>(joystick) != 0)
    {
        stickAssignPushButton->setEnabled(false);
        stickAssignPushButton->setVisible(false);
    }
#endif

    connect(loadButton, SIGNAL(clicked()), this, SLOT(openConfigFileDialog()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfigFile()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetJoystick()));
    connect(namesPushButton, SIGNAL(clicked()), this, SLOT(toggleNames()));
    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));
    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAsConfig()));
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
    connect(quickSetPushButton, SIGNAL(clicked()), this, SLOT(showQuickSetDialog()));
}

void JoyTabWidget::openConfigFileDialog()
{
    QString filename;
    QString lookupDir = QDir::homePath();

    QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);

    QString lastProfileDir = settings.value("LastProfileDir", "").toString();
    if (!lastProfileDir.isEmpty())
    {
        QFileInfo dirinfo(lastProfileDir);
        if (dirinfo.isDir() && dirinfo.isReadable())
        {
            lookupDir = lastProfileDir;
        }
    }

    filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.xml)"));

    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (searchIndex == -1)
        {
            if (configBox->count() == 6)
            {
                configBox->removeItem(5);
            }

            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
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

        settings.setValue("LastProfileDir", fileinfo.absoluteDir().absolutePath());
    }
}

void JoyTabWidget::fillButtons()
{
    /*
    QWidget *child;
    //QList<QPushButton*> list = old_layout->findChildren<QPushButton*>();
    //qDeleteAll(list);
    while (gridLayout && gridLayout->count() > 0)
    {
        child = gridLayout->takeAt(0)->widget();
        gridLayout->removeWidget (child);
        //child->deleteLater();
        delete child;
        child = 0;
    }
    */

    for (int i=0; i < Joystick::NUMBER_JOYSETS; i++)
    {
        int row = 0;
        int column = 0;

        QWidget *child = 0;
        QGridLayout *current_layout = 0;
        switch (i)
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

        while (current_layout && current_layout->count() > 0)
        {
            child = current_layout->takeAt(0)->widget();
            current_layout->removeWidget (child);
            delete child;
            child = 0;
        }

        connect (joystick, SIGNAL(setChangeActivated(int)), this, SLOT(changeCurrentSet(int)));

        QGridLayout *stickGrid = 0;
        QGroupBox *stickGroup = 0;
        int stickGridColumn = 0;
        int stickGridRow = 0;
        for (int j=0; j < joystick->getNumberSticks(); j++)
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

            JoyControlStick *stick = joystick->getSetJoystick(i)->getJoyStick(j);
            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
            QGridLayout *tempalayout = new QGridLayout();
            QWidget *attemp = new QWidget(stickGroup);

            JoyControlStickButton *button = 0;
            JoyControlStickButtonPushButton *pushbutton = 0;
            if (stick->getJoyMode() == JoyControlStick::EightWayMode)
            {
                button = stickButtons->value(JoyControlStick::StickLeftUp);
                pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 0);
            }

            button = stickButtons->value(JoyControlStick::StickUp);
            pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 0, 1);

            if (stick->getJoyMode() == JoyControlStick::EightWayMode)
            {
                button = stickButtons->value(JoyControlStick::StickRightUp);
                pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 2);
            }

            button = stickButtons->value(JoyControlStick::StickLeft);
            pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 0);

            JoyControlStickPushButton *stickWidget = new JoyControlStickPushButton(stick, displayingNames, attemp);
            stickWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
            connect(stickWidget, SIGNAL(clicked()), this, SLOT(showStickDialog()));
            connect(namesPushButton, SIGNAL(clicked()), stickWidget, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(stickWidget, 1, 1);

            button = stickButtons->value(JoyControlStick::StickRight);
            pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 2);

            if (stick->getJoyMode() == JoyControlStick::EightWayMode)
            {
                button = stickButtons->value(JoyControlStick::StickLeftDown);
                pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 0);
            }

            button = stickButtons->value(JoyControlStick::StickDown);
            pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 2, 1);

            if (stick->getJoyMode() == JoyControlStick::EightWayMode)
            {
                button = stickButtons->value(JoyControlStick::StickRightDown);
                pushbutton = new JoyControlStickButtonPushButton(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 2);
            }

            if (stickGridColumn > 1)
            {
                stickGridColumn = 0;
                stickGridRow++;
            }

            attemp->setLayout(tempalayout);
            stickGrid->addWidget(attemp, stickGridRow, stickGridColumn);
            stickGridColumn++;
        }

        if (stickGroup)
        {
            QSpacerItem *tempspacer = new QSpacerItem(10, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);
            QVBoxLayout *tempvbox = new QVBoxLayout();
            tempvbox->addLayout(stickGrid);
            tempvbox->addItem(tempspacer);
            stickGroup->setLayout(tempvbox);
            current_layout->addWidget(stickGroup, row, column, 1, 2);
        }

        row++;
        column = 0;

        QGridLayout *hatGrid = 0;
        QGroupBox *hatGroup = 0;
        int hatGridColumn = 0;
        int hatGridRow = 0;
        for (int j=0; j < joystick->getNumberHats(); j++)
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

            JoyDPad *dpad = joystick->getSetJoystick(i)->getJoyDPad(j);
            QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

            QGridLayout *tempalayout = new QGridLayout();
            QWidget *attemp = new QWidget(hatGroup);

            JoyDPadButton *button = 0;
            JoyDPadButtonWidget *pushbutton = 0;
            if (dpad->getJoyMode() == JoyDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadLeftUp);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 0);
            }

            button = buttons->value(JoyDPadButton::DpadUp);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 0, 1);

            if (dpad->getJoyMode() == JoyDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadRightUp);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 2);
            }

            button = buttons->value(JoyDPadButton::DpadLeft);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 0);

            DPadPushButton *dpadpushbutton = new DPadPushButton(dpad, displayingNames, attemp);
            dpadpushbutton->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
            connect(dpadpushbutton, SIGNAL(clicked()), this, SLOT(showDPadDialog()));
            connect(namesPushButton, SIGNAL(clicked()), dpadpushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(dpadpushbutton, 1, 1);

            button = buttons->value(JoyDPadButton::DpadRight);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 2);

            if (dpad->getJoyMode() == JoyDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadLeftDown);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 0);
            }

            button = buttons->value(JoyDPadButton::DpadDown);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 2, 1);

            if (dpad->getJoyMode() == JoyDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadRightDown);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 2);
            }

            if (hatGridColumn > 1)
            {
                hatGridColumn = 0;
                hatGridRow++;
            }

            attemp->setLayout(tempalayout);
            hatGrid->addWidget(attemp, hatGridRow, hatGridColumn);
            hatGridColumn++;
        }

        for (int j=0; j < joystick->getNumberVDPads(); j++)
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

            VDPad *vdpad = joystick->getSetJoystick(i)->getVDPad(j);
            QHash<int, JoyDPadButton*> *buttons = vdpad->getButtons();

            QGridLayout *tempalayout = new QGridLayout();
            QWidget *attemp = new QWidget(hatGroup);

            JoyDPadButton *button = 0;
            JoyDPadButtonWidget *pushbutton = 0;
            if (vdpad->getJoyMode() == VDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadLeftUp);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 0);
            }

            button = buttons->value(JoyDPadButton::DpadUp);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 0, 1);

            if (vdpad->getJoyMode() == VDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadRightUp);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 0, 2);
            }

            button = buttons->value(JoyDPadButton::DpadLeft);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 0);

            DPadPushButton *dpadpushbutton = new DPadPushButton(vdpad, displayingNames, attemp);
            dpadpushbutton->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
            connect(dpadpushbutton, SIGNAL(clicked()), this, SLOT(showDPadDialog()));
            connect(namesPushButton, SIGNAL(clicked()), dpadpushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(dpadpushbutton, 1, 1);

            button = buttons->value(JoyDPadButton::DpadRight);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 1, 2);

            if (vdpad->getJoyMode() == VDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadLeftDown);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 0);
            }

            button = buttons->value(JoyDPadButton::DpadDown);
            pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
            connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
            connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
            tempalayout->addWidget(pushbutton, 2, 1);

            if (vdpad->getJoyMode() == VDPad::EightWayMode)
            {
                button = buttons->value(JoyDPadButton::DpadRightDown);
                pushbutton = new JoyDPadButtonWidget(button, displayingNames, attemp);
                connect(pushbutton, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), pushbutton, SLOT(toggleNameDisplay()));
                tempalayout->addWidget(pushbutton, 2, 2);
            }

            if (hatGridColumn > 1)
            {
                hatGridColumn = 0;
                hatGridRow++;
            }

            attemp->setLayout(tempalayout);
            hatGrid->addWidget(attemp, hatGridRow, hatGridColumn);
            hatGridColumn++;
        }

        if (hatGroup)
        {
            QSpacerItem *tempspacer = new QSpacerItem(10, 4, QSizePolicy::Minimum, QSizePolicy::Fixed);
            QVBoxLayout *tempvbox = new QVBoxLayout();
            tempvbox->addLayout(hatGrid);
            tempvbox->addItem(tempspacer);
            hatGroup->setLayout(tempvbox);
            current_layout->addWidget(hatGroup, row, column, 1, 2);
        }

        row++;
        column = 0;

        for (int j=0; j < joystick->getNumberAxes(); j++)
        {
            JoyAxis *axis = joystick->getSetJoystick(i)->getJoyAxis(j);
            if (!axis->isPartControlStick() && axis->hasControlOfButtons())
            {
                JoyAxisWidget *axisWidget = new JoyAxisWidget(axis, displayingNames, this);
                axisWidget->setText(axis->getName());
                axisWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                axisWidget->setMinimumSize(200, 24);

                connect(axisWidget, SIGNAL(clicked()), this, SLOT(showAxisDialog()));
                connect(namesPushButton, SIGNAL(clicked()), axisWidget, SLOT(toggleNameDisplay()));

                if (column > 1)
                {
                    column = 0;
                    row++;
                }
                current_layout->addWidget(axisWidget, row, column);
                column++;
            }
        }

        for (int j=0; j < joystick->getNumberButtons(); j++)
        {
            JoyButton *button = joystick->getSetJoystick(i)->getJoyButton(j);
            if (button && !button->isPartVDPad())
            {
                JoyButtonWidget *buttonWidget = new JoyButtonWidget (button, displayingNames, this);
                buttonWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                buttonWidget->setText(buttonWidget->text());
                buttonWidget->setMinimumSize(200, 24);

                connect(buttonWidget, SIGNAL(clicked()), this, SLOT(showButtonDialog()));
                connect(namesPushButton, SIGNAL(clicked()), buttonWidget, SLOT(toggleNameDisplay()));

                if (column > 1)
                {
                    column = 0;
                    row++;
                }

                current_layout->addWidget(buttonWidget, row, column);
                column++;
            }
        }
    }
}

void JoyTabWidget::showButtonDialog()
{
    QObject *sender = QObject::sender();
    JoyButtonWidget *buttonWidget = (JoyButtonWidget*)sender;
    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, this);
    dialog->show();
}

void JoyTabWidget::showAxisDialog()
{
    QObject *sender = QObject::sender();
    JoyAxisWidget *axisWidget = (JoyAxisWidget*) sender;
    JoyAxis *axis = axisWidget->getAxis();

    axisDialog = new AxisEditDialog (axis, this);
    axisDialog->show();
}

void JoyTabWidget::showStickDialog()
{
    QObject *sender = QObject::sender();
    JoyControlStickPushButton *stickWidget = (JoyControlStickPushButton*) sender;
    JoyControlStick *stick = stickWidget->getStick();

    JoyControlStickEditDialog *dialog = new JoyControlStickEditDialog (stick, this);
    dialog->show();
    connect(dialog, SIGNAL(finished(int)), this, SLOT(fillButtons()));
}

void JoyTabWidget::saveConfigFile()
{
    int index = configBox->currentIndex();
    QString filename;
    if (index == 0)
    {
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), QDir::homePath(), QString("Config File (*.%1.xml)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }
    else
    {
        filename = configBox->itemData(index).value<QString> ();
    }

    if (!filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        if (fileinfo.suffix() != "xml")
        {
            filename = filename.append(".xml");
        }
        fileinfo.setFile(filename);

        XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);

        if (writer.hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(writer.getErrorString());
            msg.setModal(true);
            msg.show();
        }
        else if (writer.hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << writer.getErrorString() << endl;
        }
        else
        {
            int existingIndex = configBox->findData(fileinfo.absoluteFilePath());
            if (existingIndex == -1)
            {
                if (configBox->count() == 6)
                {
                    configBox->removeItem(5);
                }

                configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
                configBox->setCurrentIndex(1);
                saveDeviceSettings();
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
            else
            {
                configBox->setCurrentIndex(existingIndex);
                saveDeviceSettings();
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

        XMLConfigReader reader;
        reader.setFileName(filename);
        reader.configJoystick(joystick);

        if (reader.hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(reader.getErrorString());
            msg.setModal(true);
            msg.show();
        }
        else if (reader.hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << reader.getErrorString() << endl;
        }

        fillButtons();
    }
    else
    {
        removeCurrentButtons();
        joystick->reset();
        fillButtons();
    }
}

void JoyTabWidget::saveAsConfig()
{
    int index = configBox->currentIndex();
    QString filename;
    if (index == 0)
    {
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), QDir::homePath(), QString("Config File (*.%1.xml)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }
    else
    {
        QString configPath = configBox->itemData(index).toString();
        QFileInfo temp(configPath);
        QString tempfilename = QFileDialog::getSaveFileName(this, tr("Save Config"), temp.absoluteDir().absolutePath(), QString("Config File (*.%1.xml)").arg(joystick->getXmlName()));
        if (!tempfilename.isEmpty())
        {
            filename = tempfilename;
        }
    }

    if (!filename.isEmpty())
    {
        QFileInfo fileinfo(filename);
        if (fileinfo.suffix() != "xml")
        {
            filename = filename.append(".xml");
        }
        fileinfo.setFile(filename);

        XMLConfigWriter writer;
        writer.setFileName(fileinfo.absoluteFilePath());
        writer.write(joystick);

        if (writer.hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(writer.getErrorString());
            msg.setModal(true);
            msg.show();
        }
        else if (writer.hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << writer.getErrorString() << endl;
        }
        else
        {
            int existingIndex = configBox->findData(fileinfo.absoluteFilePath());
            if (existingIndex == -1)
            {
                if (configBox->count() == 6)
                {
                    configBox->removeItem(5);
                }

                configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
                configBox->setCurrentIndex(1);
                saveDeviceSettings();
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
            else
            {
                configBox->setCurrentIndex(existingIndex);
                saveDeviceSettings();
                emit joystickConfigChanged(joystick->getJoyNumber());
            }
        }
    }
}

void JoyTabWidget::changeJoyConfig(int index)
{
    QString filename;

    if (index > 0)
    {
        filename = configBox->itemData(index).toString();
    }

    if (!filename.isEmpty())
    {
        removeCurrentButtons();

        XMLConfigReader reader;
        reader.setFileName(filename);
        reader.configJoystick(joystick);

        if (reader.hasError() && this->window()->isEnabled())
        {
            QMessageBox msg;
            msg.setStandardButtons(QMessageBox::Close);
            msg.setText(reader.getErrorString());
            msg.setModal(true);
            msg.show();
        }
        else if (reader.hasError() && !this->window()->isEnabled())
        {
            QTextStream error(stderr);
            error << reader.getErrorString() << endl;
        }

        fillButtons();

    }
    else if (index == 0)
    {
        removeCurrentButtons();
        joystick->reset();
        fillButtons();
        emit joystickRefreshRequested(joystick);
    }
}

void JoyTabWidget::saveSettings(QSettings *settings)
{
    QString filename = "";
    QString lastfile = "";

    int index = configBox->currentIndex();
    int currentjoy = 1;

    QString controlGUIDPrefix = QString("Controller%1").arg(joystick->getGUIDString());
    QString controlGUIDString = QString("Controller%1ConfigFile%2").arg(joystick->getGUIDString());
    QString controlGUIDLastSelected = QString("Controller%1LastSelected").arg(joystick->getGUIDString());

    QString controlSDLNamePrefix = QString("Controller%1").arg(joystick->getSDLName());
    QString controlSDLNameString = QString("Controller%1ConfigFile%2").arg(joystick->getSDLName());
    QString controlSDLNameLastSelected = QString("Controller%1LastSelected").arg(joystick->getSDLName());

    // Remove current settings for a controller
    QStringList tempkeys = settings->allKeys();
    QStringListIterator iter(tempkeys);
    while (iter.hasNext())
    {
        QString tempstring = iter.next();
        if (tempstring.startsWith(controlGUIDPrefix))
        {
            settings->remove(tempstring);
        }
        else if (tempstring.startsWith(controlSDLNamePrefix))
        {
            settings->remove(tempstring);
        }
    }

    // Output currently selected profile as first profile on the list
    if (index != 0)
    {
        filename = lastfile = configBox->itemData(index).toString();
        if (!joystick->getGUIDString().isEmpty())
        {
            settings->setValue(controlGUIDString.arg(currentjoy), filename);
        }
        else if (!joystick->getSDLName().isEmpty())
        {
            settings->setValue(controlSDLNameString.arg(currentjoy), filename);
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

           if (!joystick->getGUIDString().isEmpty())
           {
               settings->setValue(controlGUIDString.arg(currentjoy), filename);
           }
           else if (!joystick->getSDLName().isEmpty())
           {
               settings->setValue(controlSDLNameString.arg(currentjoy), filename);
           }

           currentjoy++;
        }
    }

    if (!joystick->getGUIDString().isEmpty())
    {
        settings->setValue(controlGUIDLastSelected, lastfile);
    }
    else if (!joystick->getSDLName().isEmpty())
    {
        settings->setValue(controlSDLNameLastSelected, lastfile);
    }
}

void JoyTabWidget::loadSettings(QSettings *settings, bool forceRefresh)
{
    disconnect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));

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

    settings->beginGroup("Controllers");
    QString controlGUIDString = QString("Controller%1ConfigFile%2").arg(joystick->getGUIDString());
    QString controlGUIDLastSelected = QString("Controller%1LastSelected").arg(joystick->getGUIDString());

    QString controlSDLNameString = QString("Controller%1ConfigFile%2").arg(joystick->getSDLName());
    QString controlSDLNameLastSelected = QString("Controller%1LastSelected").arg(joystick->getSDLName());

    for (int i=1; i <= 5; i++)
    {
        QString tempfilepath;

        if (!joystick->getGUIDString().isEmpty())
        {
            tempfilepath = settings->value(controlGUIDString.arg(i), "").toString();
        }
        else if (!joystick->getSDLName().isEmpty())
        {
            tempfilepath = settings->value(controlSDLNameString.arg(i), "").toString();
        }

        if (!tempfilepath.isEmpty())
        {
            QFileInfo fileInfo(tempfilepath);
            if (configBox->findData(fileInfo.absoluteFilePath()) == -1)
            {
                configBox->addItem(fileInfo.baseName(), fileInfo.absoluteFilePath());
            }
        }
    }

    connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeJoyConfig(int)));

    QString lastfile;
    if (!joystick->getGUIDString().isEmpty())
    {
        lastfile = settings->value(controlGUIDLastSelected, "").toString();
    }
    else if (!joystick->getSDLName().isEmpty())
    {
        lastfile = settings->value(controlSDLNameLastSelected, "").toString();
    }

    settings->endGroup();

    if (!lastfile.isEmpty())
    {
        int lastindex = configBox->findData(lastfile);
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
    QHash<int, QString> *temp = new QHash<int, QString> ();
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
    connect(dialog, SIGNAL(finished(int)), this, SLOT(fillButtons()));
    dialog->show();
}

void JoyTabWidget::loadConfigFile(QString fileLocation)
{
    QFileInfo fileinfo(fileLocation);
    if (fileinfo.exists() && fileinfo.suffix() == "xml")
    {
        int searchIndex = configBox->findData(fileinfo.absoluteFilePath());
        if (searchIndex == -1)
        {
            if (configBox->count() == 6)
            {
                configBox->removeItem(5);
            }
            configBox->insertItem(1, fileinfo.baseName(), fileinfo.absoluteFilePath());
            configBox->setCurrentIndex(1);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
        else
        {
            configBox->setCurrentIndex(searchIndex);
            emit joystickConfigChanged(joystick->getJoyNumber());
        }
    }
}

void JoyTabWidget::openStickButtonDialog()
{
    JoyControlStickButtonPushButton *pushbutton = static_cast<JoyControlStickButtonPushButton*> (sender());
    ButtonEditDialog *dialog = new ButtonEditDialog(pushbutton->getButton(), this);
    dialog->show();
}

void JoyTabWidget::showDPadDialog()
{
    DPadPushButton *pushbutton = static_cast<DPadPushButton*> (sender());
    DPadEditDialog *dialog = new DPadEditDialog(pushbutton->getDPad(), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(fillButtons()));
}

void JoyTabWidget::showQuickSetDialog()
{
    QuickSetDialog *dialog = new QuickSetDialog(joystick, this);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(fillButtons()));
    dialog->show();
}

void JoyTabWidget::removeCurrentButtons()
{
    for (int i=0; i < Joystick::NUMBER_JOYSETS; i++)
    {
        QLayoutItem *child = 0;
        QGridLayout *current_layout = 0;
        switch (i)
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
            child = 0;
        }
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
        saveDeviceSettings();
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

void JoyTabWidget::saveDeviceSettings()
{
    QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    settings.beginGroup("Controllers");
    saveSettings(&settings);
    settings.endGroup();
}

void JoyTabWidget::loadDeviceSettings()
{
    QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    //settings.beginGroup("Controllers");
    loadSettings(&settings);
    settings.endGroup();
}

bool JoyTabWidget::isDisplayingNames()
{
    return displayingNames;
}
