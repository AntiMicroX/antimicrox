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


#include "virtualkeyboardmousewidget.h"

#include "messagehandler.h"
#include "virtualkeypushbutton.h"
#include "virtualmousepushbutton.h"
#include "mousedialog/mousebuttonsettingsdialog.h"
#include "joybutton.h"
#include "joybuttonslot.h"
#include "event.h"
#include "antkeymapper.h"
#include "quicksetdialog.h"
#include "buttoneditdialog.h"

#ifdef Q_OS_UNIX
#include "x11extras.h"
#include <X11/Xlib.h>
#elif defined(Q_OS_WIN)
#include "winextras.h"
#include <windows.h>
#endif

#include <SDL2/SDL_power.h>

#include <QFont>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QListIterator>
#include <QLocale>
#include <QList>
#include <QListIterator>
#include <QPushButton>
#include <QResizeEvent>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

QHash<QString, QString> VirtualKeyboardMouseWidget::topRowKeys = QHash<QString, QString> ();

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(InputDevice *joystick, ButtonEditDialogHelper* helper, QuickSetDialog* quickSetDialog, JoyButton* button, QWidget *parent) :
    QTabWidget(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->joystick = joystick;
    this->helper = helper;
    this->withoutQuickSetDialog = (button != nullptr);
    lastPressedBtn = button;
    currentQuickDialog = quickSetDialog;
    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    isLaptopDevice = isLaptop();
    noneButton = createNoneKey();

    populateTopRowKeys();

    this->addTab(keyboardTab, trUtf8("Keyboard"));
    this->addTab(mouseTab, trUtf8("Mouse"));
    this->setTabPosition(QTabWidget::South);

    setupVirtualKeyboardLayout();
    setupMouseControlLayout();
    establishVirtualKeyboardSingleSignalConnections();
    establishVirtualMouseSignalConnections();

    QTimer::singleShot(0, this, SLOT(setButtonFontSizes()));

    connect(mouseSettingsPushButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::openMouseSettingsDialog);
}


VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(QWidget *parent) :
    QTabWidget(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput); 

    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    isLaptopDevice = isLaptop();
    noneButton = createNoneKey();
    withoutQuickSetDialog = false;
    lastPressedBtn = nullptr;

    populateTopRowKeys();

    this->addTab(keyboardTab, trUtf8("Keyboard"));
    this->addTab(mouseTab, trUtf8("Mouse"));
    this->setTabPosition(QTabWidget::South);

    setupVirtualKeyboardLayout();
    setupMouseControlLayout();
    establishVirtualKeyboardSingleSignalConnections();
    establishVirtualMouseSignalConnections();

    QTimer::singleShot(0, this, SLOT(setButtonFontSizes()));
}


bool VirtualKeyboardMouseWidget::is_numlock_activated()
{
#ifdef Q_OS_WIN
    short status = GetKeyState(VK_NUMLOCK);
    return status == 1;
#endif

#ifdef Q_OS_UNIX
    Display *dpy = XOpenDisplay(X11Extras::getInstance()->getEnvVariable("DISPLAY"));
    XKeyboardState x;
    XGetKeyboardControl(dpy, &x);
    XCloseDisplay(dpy);
    return x.led_mask & 2;
#endif
}

bool VirtualKeyboardMouseWidget::isLaptop()
{
    int secs, pct;

    if (SDL_GetPowerInfo(&secs, &pct) == SDL_POWERSTATE_UNKNOWN) return false;
    else return true;
}


void VirtualKeyboardMouseWidget::setupVirtualKeyboardLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(keyboardTab);
    QVBoxLayout *tempMainKeyLayout = setupMainKeyboardLayout();
    QVBoxLayout *tempAuxKeyLayout = new QVBoxLayout();
    QVBoxLayout *tempNumKeyPadLayout = new QVBoxLayout();

   /* if (is_numlock_activated())
    {
        tempNumKeyPadLayout = setupKeyboardNumPadLayout();
    }
    else
    {*/
        QPushButton *othersKeysButton = createOtherKeysMenu();

        tempNumKeyPadLayout->addWidget(noneButton);
        tempNumKeyPadLayout->addWidget(othersKeysButton);
        tempNumKeyPadLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    //}

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->addLayout(tempMainKeyLayout);

    if (!isLaptopDevice)
    {
        tempAuxKeyLayout = setupAuxKeyboardLayout();
        tempHBoxLayout->addLayout(tempAuxKeyLayout);
    }
    else
    {
        delete tempAuxKeyLayout;
    }

    tempHBoxLayout->addLayout(tempNumKeyPadLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

QVBoxLayout *VirtualKeyboardMouseWidget::setupMainKeyboardLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);

    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(0);

    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();
    if (isLaptopDevice) finalVBoxLayout->setSpacing(0);

    tempHBoxLayout->addWidget(createNewKey("Escape"));
    if (!isLaptopDevice) tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F1"));
    tempHBoxLayout->addWidget(createNewKey("F2"));
    tempHBoxLayout->addWidget(createNewKey("F3"));
    tempHBoxLayout->addWidget(createNewKey("F4"));
    if (!isLaptopDevice) tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F5"));
    tempHBoxLayout->addWidget(createNewKey("F6"));
    tempHBoxLayout->addWidget(createNewKey("F7"));
    tempHBoxLayout->addWidget(createNewKey("F8"));
    if (!isLaptopDevice) tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F9"));
    tempHBoxLayout->addWidget(createNewKey("F10"));
    tempHBoxLayout->addWidget(createNewKey("F11"));
    tempHBoxLayout->addWidget(createNewKey("F12"));

    if (isLaptopDevice)
    {
        tempHBoxLayout->addWidget(createNewKey("Print"));
        tempHBoxLayout->addWidget(createNewKey("Pause"));
        tempHBoxLayout->addWidget(createNewKey("Delete"));
    }

    finalVBoxLayout->addLayout(tempHBoxLayout);

    if (!isLaptopDevice) finalVBoxLayout->addSpacerItem(new QSpacerItem(20, 35, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("grave"));

    for (int i=1; i <= 9; i++)
    {
        tempHBoxLayout->addWidget(createNewKey(QString::number(i)));
    }

    tempHBoxLayout->addWidget(createNewKey("0"));
    tempHBoxLayout->addWidget(createNewKey("minus"));
    tempHBoxLayout->addWidget(createNewKey("equal"));
    tempHBoxLayout->addWidget(createNewKey("BackSpace"));

    if (isLaptopDevice) {
        tempHBoxLayout->addWidget(createNewKey("Home"));
    }

    tempVBoxLayout->addLayout(tempHBoxLayout);

    QVBoxLayout *tempMiddleVLayout = new QVBoxLayout();
    tempMiddleVLayout->setSpacing(0);

    QHBoxLayout *tempMiddleHLayout = new QHBoxLayout();
    tempHBoxLayout = new QHBoxLayout();

    tempHBoxLayout->addWidget(createNewKey("Tab"));
    if (!isLaptopDevice) tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 30, QSizePolicy::Fixed));
    tempHBoxLayout->addWidget(createNewKey("q"));
    tempHBoxLayout->addWidget(createNewKey("w"));
    tempHBoxLayout->addWidget(createNewKey("e"));
    tempHBoxLayout->addWidget(createNewKey("r"));
    tempHBoxLayout->addWidget(createNewKey("t"));
    tempHBoxLayout->addWidget(createNewKey("y"));
    tempHBoxLayout->addWidget(createNewKey("u"));
    tempHBoxLayout->addWidget(createNewKey("i"));
    tempHBoxLayout->addWidget(createNewKey("o"));
    tempHBoxLayout->addWidget(createNewKey("p"));
    tempHBoxLayout->addWidget(createNewKey("bracketleft"));
    tempHBoxLayout->addWidget(createNewKey("bracketright"));

    if ((QLocale::system().language() != QLocale::French) &&
        (QLocale::system().language() != QLocale::German))
    {
        tempHBoxLayout->addWidget(createNewKey("backslash"));
    }

    if (isLaptopDevice)
    {
        tempHBoxLayout->addWidget(createNewKey("Prior"));
        tempVBoxLayout->addLayout(tempHBoxLayout);
    }
    else
    {
        tempMiddleVLayout->addLayout(tempHBoxLayout);
    }

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Caps_Lock"));
    tempHBoxLayout->addWidget(createNewKey("a"));
    tempHBoxLayout->addWidget(createNewKey("s"));
    tempHBoxLayout->addWidget(createNewKey("d"));
    tempHBoxLayout->addWidget(createNewKey("f"));
    tempHBoxLayout->addWidget(createNewKey("g"));
    tempHBoxLayout->addWidget(createNewKey("h"));
    tempHBoxLayout->addWidget(createNewKey("j"));
    tempHBoxLayout->addWidget(createNewKey("k"));
    tempHBoxLayout->addWidget(createNewKey("l"));
    tempHBoxLayout->addWidget(createNewKey("semicolon"));
    tempHBoxLayout->addWidget(createNewKey("apostrophe"));

    if ((QLocale::system().language() == QLocale::French) ||
        (QLocale::system().language() == QLocale::German))
    {
        tempHBoxLayout->addWidget(createNewKey("asterisk"));
    }

    if (!isLaptopDevice)
    {
        tempMiddleVLayout->addLayout(tempHBoxLayout);
        tempMiddleHLayout->addLayout(tempMiddleVLayout);
        tempMiddleHLayout->setSpacing(0);
        tempMiddleHLayout->addWidget(createNewKey("Return"));

        tempVBoxLayout->addLayout(tempMiddleHLayout);
    }
    else
    {
        tempHBoxLayout->addWidget(createNewKey("Return"));
        tempHBoxLayout->addWidget(createNewKey("Next"));

        tempVBoxLayout->addLayout(tempHBoxLayout);

        delete tempMiddleHLayout;
        delete tempMiddleVLayout;
    }

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Shift_L"));

    if (QLocale::system().language() == QLocale::French)
    {
        tempHBoxLayout->addWidget(createNewKey("less"));
    }

    tempHBoxLayout->addWidget(createNewKey("z"));
    tempHBoxLayout->addWidget(createNewKey("x"));
    tempHBoxLayout->addWidget(createNewKey("c"));
    tempHBoxLayout->addWidget(createNewKey("v"));
    tempHBoxLayout->addWidget(createNewKey("b"));
    tempHBoxLayout->addWidget(createNewKey("n"));
    tempHBoxLayout->addWidget(createNewKey("m"));
    tempHBoxLayout->addWidget(createNewKey("comma"));
    tempHBoxLayout->addWidget(createNewKey("period"));
    tempHBoxLayout->addWidget(createNewKey("slash"));
    tempHBoxLayout->addWidget(createNewKey("Shift_R"));

    if (isLaptopDevice)
    {
        tempHBoxLayout->addWidget(createNewKey("Up"));
        tempHBoxLayout->addWidget(createNewKey("End"));
    }

    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Control_L"));
    tempHBoxLayout->addWidget(createNewKey("Super_L"));
    tempHBoxLayout->addWidget(createNewKey("Alt_L"));
    tempHBoxLayout->addWidget(createNewKey("space"));

    VirtualKeyPushButton* altR = createNewKey("Alt_R");

    if (altR->getKeycode() <= 0)
    {
        tempHBoxLayout->addWidget(createNewKey("ISO_Level3_Shift"));
        delete altR;
    }
    else
    {
        tempHBoxLayout->addWidget(altR);
    }

    tempHBoxLayout->addWidget(createNewKey("Menu"));
    tempHBoxLayout->addWidget(createNewKey("Control_R"));

    if (isLaptopDevice)
    {
        tempHBoxLayout->addWidget(createNewKey("Left"));
        tempHBoxLayout->addWidget(createNewKey("Down"));
        tempHBoxLayout->addWidget(createNewKey("Right"));
    }

    tempVBoxLayout->addLayout(tempHBoxLayout);

    if (!isLaptopDevice)
    {
        tempVBoxLayout->setStretch(0, 1);
        tempVBoxLayout->setStretch(1, 2);
        tempVBoxLayout->setStretch(2, 1);
        tempVBoxLayout->setStretch(3, 1);
    }

    finalVBoxLayout->addLayout(tempVBoxLayout);

    if (!isLaptopDevice)
    {
        finalVBoxLayout->setStretch(0, 1);
        finalVBoxLayout->setStretch(1, 0);
        finalVBoxLayout->setStretch(2, 2);
    }

    return finalVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupAuxKeyboardLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();

    tempHBoxLayout->setSpacing(0);
    tempVBoxLayout->setSpacing(0);
    tempGridLayout->setSpacing(0);

    tempHBoxLayout->addWidget(createNewKey("Print"));
    tempHBoxLayout->addWidget(createNewKey("Scroll_Lock"));
    tempHBoxLayout->addWidget(createNewKey("Pause"));

    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 45, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout->addWidget(createNewKey("Insert"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Home"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Prior"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("Delete"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("End"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Next"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 35, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout = new QGridLayout();
    tempGridLayout->addWidget(createNewKey("Up"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Left"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Down"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Right"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    return tempVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupKeyboardNumPadLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);

    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(0);

    QGridLayout *tempGridLayout = new QGridLayout();
    tempGridLayout->setSpacing(0);

    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();
    finalVBoxLayout->setSpacing(0);

    QPushButton *othersKeysButton = createOtherKeysMenu();

    finalVBoxLayout->addWidget(noneButton);
    finalVBoxLayout->addWidget(othersKeysButton);
    finalVBoxLayout->setStretchFactor(noneButton, 1);
    finalVBoxLayout->setStretchFactor(othersKeysButton, 1);
    finalVBoxLayout->addSpacerItem(new QSpacerItem(0, 29, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Num_Lock"));
    tempHBoxLayout->addWidget(createNewKey("KP_Divide"));
    tempHBoxLayout->addWidget(createNewKey("KP_Multiply"));
    tempHBoxLayout->addWidget(createNewKey("KP_Subtract"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("KP_7"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_8"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_9"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_4"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_5"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_6"), 2, 3, 1, 1);
    tempHBoxLayout->addLayout(tempGridLayout);
    tempHBoxLayout->addWidget(createNewKey("KP_Add"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);

    tempGridLayout = new QGridLayout();
    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("KP_1"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_2"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_3"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_0"), 2, 1, 1, 2);
    tempGridLayout->addWidget(createNewKey("KP_Decimal"), 2, 3, 1, 1);

    tempHBoxLayout->addLayout(tempGridLayout);
    tempHBoxLayout->addWidget(createNewKey("KP_Enter"));

    tempVBoxLayout->addLayout(tempHBoxLayout);
    finalVBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->setStretchFactor(tempVBoxLayout, 8);

    return finalVBoxLayout;
}

void VirtualKeyboardMouseWidget::setupMouseControlLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(mouseTab);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    VirtualMousePushButton *pushButton = new VirtualMousePushButton(trUtf8("Left", "Mouse"), JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    pushButton = new VirtualMousePushButton(trUtf8("Up", "Mouse"), JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    QHBoxLayout *tempInnerHBoxLayout = new QHBoxLayout();
    pushButton = new VirtualMousePushButton(trUtf8("Left Button", "Mouse"), 1, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton(trUtf8("Middle Button", "Mouse"), 2, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton(trUtf8("Right Button", "Mouse"), 3, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addLayout(tempInnerHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton = new VirtualMousePushButton(trUtf8("Wheel Up", "Mouse"), 4, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 1, 2, 1, 1);
    pushButton = new VirtualMousePushButton(trUtf8("Wheel Left", "Mouse"), 6, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 1, 1, 1);
    pushButton = new VirtualMousePushButton(trUtf8("Wheel Right", "Mouse"), 7, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 3, 1, 1);
    pushButton = new VirtualMousePushButton(trUtf8("Wheel Down", "Mouse"), 5, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 3, 2, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton = new VirtualMousePushButton(trUtf8("Down", "Mouse"), JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);

    tempVBoxLayout->setStretch(0, 1);
    tempVBoxLayout->setStretch(2, 1);
    tempVBoxLayout->setStretch(4, 3);
    tempVBoxLayout->setStretch(6, 1);

    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    pushButton = new VirtualMousePushButton(trUtf8("Right", "Mouse"), JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(20);
#ifdef Q_OS_WIN
    pushButton = new VirtualMousePushButton(trUtf8("Button 4", "Mouse"), 8, JoyButtonSlot::JoyMouseButton, this);
#elif defined(Q_OS_UNIX)
    pushButton = new VirtualMousePushButton(trUtf8("Mouse 8", "Mouse"), 8, JoyButtonSlot::JoyMouseButton, this);
#endif

    pushButton->setMinimumHeight(40);
    tempVBoxLayout->addWidget(pushButton);
#ifdef Q_OS_WIN
    pushButton = new VirtualMousePushButton(trUtf8("Button 5", "Mouse"), 9, JoyButtonSlot::JoyMouseButton, this);
#elif defined(Q_OS_UNIX)
    pushButton = new VirtualMousePushButton(trUtf8("Mouse 9", "Mouse"), 9, JoyButtonSlot::JoyMouseButton, this);
#endif

    pushButton->setMinimumHeight(40);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(20);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mouseSettingsPushButton = new QPushButton(trUtf8("Mouse Settings"), this);
    mouseSettingsPushButton->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-select")));
    tempVBoxLayout->addWidget(mouseSettingsPushButton);

    mouseSettingsPushButton->setEnabled(false);

    tempHBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

VirtualKeyPushButton* VirtualKeyboardMouseWidget::createNewKey(QString xcodestring)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "parameter 1: " << xcodestring;
    #endif

    int width = 32;
    int height = 32;
    QFont font1;
    font1.setPointSize(8);
    font1.setBold(true);

    VirtualKeyPushButton *pushButton = new VirtualKeyPushButton(xcodestring, this);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pushButton->setSizePolicy(sizePolicy);

    if (xcodestring == "space")
    {
        width = 131;
    }
    else if (xcodestring == "Tab")
    {
        width = 43;
    }
    else if (xcodestring == "Shift_L")
    {
        width = 77;
    }
    else if (xcodestring == "Shift_R")
    {
        if (isLaptopDevice) width = 59;
        else width = 95;
    }
    else if (xcodestring == "Caps_Lock")
    {
        width = 59;
    }
    else if (xcodestring == "Return")
    {
        width = 64;
        if (!isLaptopDevice) height = 64;
        pushButton->setMaximumWidth(100);
    }
    else if (xcodestring == "BackSpace")
    {
        width = 68;
    }
    else if (topRowKeys.contains(xcodestring))
    {
        width = 30;
        height = 32;
        pushButton->setMaximumSize(100, 100);
    }
    else if ((xcodestring == "Print") || (xcodestring == "Scroll_Lock") || (xcodestring == "Pause"))
    {
        width = 32;
        height = 32;
        pushButton->setMaximumSize(100, 100);
        font1.setPointSize(6);
    }
    else if ((xcodestring == "KP_Add") || (xcodestring == "KP_Enter"))
    {
        height = 56;
        width = 32;
        font1.setPointSize(5);
    }
    else if (xcodestring == "Num_Lock")
    {
        width = 32;
        height = 32;
        font1.setPointSize(5);
    }
    else if (xcodestring.startsWith("KP_"))
    {
        width = 32;
    }
    else if (xcodestring == "backslash") {

        if (!isLaptopDevice) width = 32;
        else width = 43;
    }
    else if ((xcodestring == "Down") || (xcodestring == "Left") || (xcodestring == "Right"))
    {
        if (isLaptopDevice) {
            QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
            pushButton->setSizePolicy(sizePolicy2);
            pushButton->setFixedWidth(58);
        }
    }
    else if ((xcodestring == "Control_L") || (xcodestring == "Super_L") || (xcodestring == "Alt_L") || (xcodestring == "Alt_R")|| (xcodestring == "ISO_Level3_Shift")|| (xcodestring == "Menu")|| (xcodestring == "Control_R"))
    {
        if (isLaptopDevice) width = 32;
        else width = 41;
    }
    else if ((xcodestring.startsWith("F") && (xcodestring.count() > 1)) || (xcodestring == "Print") || (xcodestring == "Escape") || (xcodestring == "Pause") || (xcodestring == "Delete"))
    {
        if (isLaptopDevice)
        {
            QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
            pushButton->setSizePolicy(sizePolicy2);
            width = 32;
            height = 25;
            font1.setPointSize(5);
        }
    }

    pushButton->setObjectName(xcodestring);
    pushButton->setMinimumSize(width, height);
    pushButton->setFont(font1);

    return pushButton;
}


QPushButton* VirtualKeyboardMouseWidget::createNoneKey()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPushButton *pushButton = new QPushButton(trUtf8("NONE"), this);
    pushButton->setMinimumSize(0, 25);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    pushButton->setSizePolicy(sizePolicy);
    QFont font1;
    font1.setBold(true);
    pushButton->setFont(font1);

    return pushButton;
}

void VirtualKeyboardMouseWidget::processSingleKeyboardSelection(int keycode, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((currentQuickDialog == nullptr) && !withoutQuickSetDialog)
    {
        currentQuickDialog = new QuickSetDialog(joystick, helper, "setAssignedSlot", keycode, alias, 0, JoyButtonSlot::JoyKeyboard, true, true, this);
        currentQuickDialog->show();
        connect(currentQuickDialog, &QuickSetDialog::finished, this, &VirtualKeyboardMouseWidget::nullifyDialogPointer);
    }
    else if (withoutQuickSetDialog)
    {
        ButtonEditDialog::getInstance()->invokeMethodLastBtn(lastPressedBtn, helper, "setAssignedSlot", keycode, alias, 0, JoyButtonSlot::JoyKeyboard, true, true, Qt::QueuedConnection, Qt::QueuedConnection, Qt::QueuedConnection);
        ButtonEditDialog::getInstance()->refreshForLastBtn();
    }
}

void VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection(int keycode, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit selectionMade(keycode, alias);
}

void VirtualKeyboardMouseWidget::processSingleMouseSelection(JoyButtonSlot *tempslot)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((currentQuickDialog == nullptr) && !withoutQuickSetDialog)
    {
        currentQuickDialog = new QuickSetDialog(joystick, helper, "setAssignedSlot", tempslot->getSlotCode(), -1, -1, tempslot->getSlotMode(), true, true, this);
        currentQuickDialog->show();
        connect(currentQuickDialog, &QuickSetDialog::finished, this, &VirtualKeyboardMouseWidget::nullifyDialogPointer);
    }
    else if (withoutQuickSetDialog)
    {
        ButtonEditDialog::getInstance()->invokeMethodLastBtn(lastPressedBtn, helper, "setAssignedSlot", tempslot->getSlotCode(), -1, -1, tempslot->getSlotMode(), true, true, Qt::QueuedConnection, Qt::QueuedConnection, Qt::QueuedConnection);
        ButtonEditDialog::getInstance()->refreshForLastBtn();
    }
}

void VirtualKeyboardMouseWidget::processAdvancedMouseSelection(JoyButtonSlot *tempslot)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit selectionMade(tempslot);
}

void VirtualKeyboardMouseWidget::populateTopRowKeys()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (topRowKeys.isEmpty())
    {
        topRowKeys.insert("Escape", "Escape");
        topRowKeys.insert("F1", "F1");
        topRowKeys.insert("F2", "F2");
        topRowKeys.insert("F3", "F3");
        topRowKeys.insert("F4", "F4");
        topRowKeys.insert("F5", "F5");
        topRowKeys.insert("F6", "F6");
        topRowKeys.insert("F7", "F7");
        topRowKeys.insert("F8", "F8");
        topRowKeys.insert("F9", "F9");
        topRowKeys.insert("F10", "F10");
        topRowKeys.insert("F11", "F11");
        topRowKeys.insert("F12", "F12");
    }
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, &VirtualKeyPushButton::keycodeObtained, nullptr, nullptr);
        connect(keybutton, &VirtualKeyPushButton::keycodeObtained, this, &VirtualKeyboardMouseWidget::processSingleKeyboardSelection);
    }

    QListIterator<QAction*> iterActions(otherKeysMenu->actions());

    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, &QAction::triggered, nullptr, nullptr);

        connect(temp, &QAction::triggered, this, [this, temp](bool checked) {
            otherKeysActionSingle(temp, checked);
        });
    }

    disconnect(noneButton, &QPushButton::clicked, nullptr, nullptr);
    connect(noneButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::clearButtonSlotsFinish);

    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "COUNT: " << newlist.count();
    #endif
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, &VirtualKeyPushButton::keycodeObtained, nullptr, nullptr);
        connect(keybutton, &VirtualKeyPushButton::keycodeObtained, this, &VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection);
    }

    QListIterator<QAction*> iterActions(otherKeysMenu->actions());

    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, &QAction::triggered, nullptr, nullptr);

        connect(temp, &QAction::triggered, this, [this, temp](bool checked) {
            otherKeysActionAdvanced(temp, checked);
        });
    }

    disconnect(noneButton, &QPushButton::clicked, nullptr, nullptr);
    connect(noneButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::clearButtonSlots);
}

void VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);

    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, nullptr, nullptr);
        connect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, this, &VirtualKeyboardMouseWidget::processSingleMouseSelection);
    }
}

void VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);

    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, nullptr, nullptr);
        connect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, this, &VirtualKeyboardMouseWidget::processAdvancedMouseSelection);
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
            QMetaObject::invokeMethod(ButtonEditDialog::getInstance()->getLastJoyButton(), "clearSlotsEventReset", Qt::BlockingQueuedConnection);

        emit selectionCleared();

    }
    else
    {
        QMessageBox::information(this, trUtf8("Last button"), trUtf8("Slots for button couldn't be cleared, because there was not any set button from keyboard for gamepad. Map at least one button from keyboard to gamepad"));
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlotsFinish()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
            QMetaObject::invokeMethod(ButtonEditDialog::getInstance()->getLastJoyButton(), "clearSlotsEventReset", Qt::BlockingQueuedConnection);

        emit selectionFinished();
    }
    else
    {
        QMessageBox::information(this, trUtf8("Last button"), trUtf8("Slots for button couldn't be cleared, because there was not any set button from keyboard for gamepad. Map at least one button from keyboard to gamepad"));
    }
}

bool VirtualKeyboardMouseWidget::isKeyboardTabVisible()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->keyboardTab->isVisible();
}

void VirtualKeyboardMouseWidget::openMouseSettingsDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseSettingsPushButton->setEnabled(false);

    // TODO instead of buttons, get pointer to static getInstance from ButtonEditDialog for last pressed button, and then getting button from public method
    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
        {
            MouseButtonSettingsDialog *dialog = new MouseButtonSettingsDialog(ButtonEditDialog::getInstance()->getLastJoyButton(), this);
            dialog->show();
            QDialog *parent = qobject_cast<QDialog*>(this->parentWidget()); // static_cast
            connect(parent, &QDialog::finished, dialog, &MouseButtonSettingsDialog::close);
            connect(dialog, &MouseButtonSettingsDialog::finished, this, &VirtualKeyboardMouseWidget::enableMouseSettingButton);
        }
        else
        {
            QMessageBox::information(this, QObject::trUtf8("Last button"), QObject::trUtf8("There isn't a last button pressed from gamepad in data. Did you set at least one button from gamepad for keyboard?"));
        }
    }
}

void VirtualKeyboardMouseWidget::enableMouseSettingButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseSettingsPushButton->setEnabled(true);
}

void VirtualKeyboardMouseWidget::disableMouseSettingButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseSettingsPushButton->setEnabled(false);
}

void VirtualKeyboardMouseWidget::resizeEvent(QResizeEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QTabWidget::resizeEvent(event);
    setButtonFontSizes();
}

// Dynamically change font size of list of push button according to the
// size of the buttons.
void VirtualKeyboardMouseWidget::setButtonFontSizes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<VirtualKeyPushButton*> buttonList = this->findChildren<VirtualKeyPushButton*>();
    QListIterator<VirtualKeyPushButton*> iter(buttonList);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *temp = iter.next();
        QFont tempFont(temp->font());
        tempFont.setPointSize(temp->calculateFontSize());
        temp->setFont(tempFont);
    }
}

QPushButton* VirtualKeyboardMouseWidget::createOtherKeysMenu()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPushButton *otherKeysPushbutton = new QPushButton(trUtf8("Others"), this);
    otherKeysPushbutton->setMinimumSize(0, 25);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    otherKeysPushbutton->setSizePolicy(sizePolicy);
    QFont font1;
    font1.setBold(true);
    otherKeysPushbutton->setFont(font1);

    otherKeysMenu = new QMenu(this);

    QAction *tempAction = nullptr;
    int temp = 0;

#ifdef Q_OS_WIN
    tempAction = new QAction(trUtf8("Applications"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Menu);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);
#endif

    tempAction = new QAction(trUtf8("Browser Back"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Back);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Favorites"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Favorites);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Forward"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Forward);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Home"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_HomePage);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Refresh"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Refresh);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Search"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Search);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Browser Stop"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Stop);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Calc"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Launch1);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Email"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_LaunchMail);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Media"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_LaunchMedia);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Media Next"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaNext);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Media Play"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaPlay);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Media Previous"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaPrevious);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Media Stop"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaStop);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Search"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Search);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Volume Down"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeDown);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Volume Mute"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeMute);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(trUtf8("Volume Up"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeUp);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    otherKeysPushbutton->setMenu(otherKeysMenu);
    return otherKeysPushbutton;
}

void VirtualKeyboardMouseWidget::otherKeysActionSingle(QAction* tempAction, bool triggered)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(triggered);

    int virtualkey = tempAction->data().toInt();
    processSingleKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}

void VirtualKeyboardMouseWidget::otherKeysActionAdvanced(QAction* tempAction, bool triggered)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(triggered);

    int virtualkey = tempAction->data().toInt();
    processAdvancedKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}


void VirtualKeyboardMouseWidget::nullifyDialogPointer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentQuickDialog != nullptr)
    {
        ButtonEditDialog::getInstance()->setUpLastJoyButton(currentQuickDialog->getLastPressedButton());
        ButtonEditDialog::getInstance()->refreshForLastBtn();
        enableMouseSettingButton();
        currentQuickDialog = nullptr;
        emit buttonDialogClosed();
    }
}

InputDevice* VirtualKeyboardMouseWidget::getJoystick() const
{
    return joystick;
}

ButtonEditDialogHelper* VirtualKeyboardMouseWidget::getHelper() const  {

    return helper;
}

QWidget* VirtualKeyboardMouseWidget::getKeyboardTab() const
{
    return keyboardTab;
}

QWidget* VirtualKeyboardMouseWidget::getMouseTab() const
{
    return mouseTab;
}

QPushButton* VirtualKeyboardMouseWidget::getNoneButton() const
{
    return noneButton;
}

QPushButton* VirtualKeyboardMouseWidget::getMouseSettingsPushButton() const
{
    return mouseSettingsPushButton;
}

QMenu* VirtualKeyboardMouseWidget::getOtherKeysMenu() const
{
    return otherKeysMenu;
}

QuickSetDialog* VirtualKeyboardMouseWidget::getCurrentQuickDialog() const
{
    return currentQuickDialog;
}
