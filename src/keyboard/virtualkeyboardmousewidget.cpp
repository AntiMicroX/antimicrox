/* antimicrox Gamepad to KB+M event mapper
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

#include "virtualkeyboardmousewidget.h"

#include "antkeymapper.h"
#include "buttoneditdialog.h"
#include "event.h"
#include "mousedialog/mousebuttonsettingsdialog.h"
#include "qtkeymapperbase.h"
#include "quicksetdialog.h"
#include "virtualkeypushbutton.h"
#include "virtualmousepushbutton.h"
#include <QtDebug>

#ifdef WITH_X11
    #include "x11extras.h"
#endif

#include <SDL2/SDL_power.h>

#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QList>
#include <QListIterator>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
//#include <QDebug>

QHash<QString, QString> VirtualKeyboardMouseWidget::topRowKeys = QHash<QString, QString>();

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(InputDevice *joystick, ButtonEditDialogHelper *helper,
                                                       bool isNumKeypad, QuickSetDialog *quickSetDialog, JoyButton *button,
                                                       QWidget *parent)
    : QTabWidget(parent)
{
    m_isNumKeypad = isNumKeypad;
    this->joystick = joystick;
    this->helper = helper;
    this->withoutQuickSetDialog = (button != nullptr);
    lastPressedBtn = button;
    currentQuickDialog = quickSetDialog;
    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    noneButton = createNoneKey();

    populateTopRowKeys();

    this->addTab(keyboardTab, tr("Keyboard"));
    this->addTab(mouseTab, tr("Mouse"));
    this->setTabPosition(QTabWidget::South);

    setupVirtualKeyboardLayout();
    setupMouseControlLayout();
    establishVirtualKeyboardSingleSignalConnections();
    establishVirtualMouseSignalConnections();

    QTimer::singleShot(0, this, SLOT(setButtonFontSizes()));

    connect(mouseSettingsPushButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::openMouseSettingsDialog);
}

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(bool isNumKeypad, QWidget *parent)
    : QTabWidget(parent)
{
    m_isNumKeypad = isNumKeypad;
    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    noneButton = createNoneKey();
    withoutQuickSetDialog = false;
    lastPressedBtn = nullptr;

    populateTopRowKeys();

    this->addTab(keyboardTab, tr("Keyboard"));
    this->addTab(mouseTab, tr("Mouse"));
    this->setTabPosition(QTabWidget::South);

    setupVirtualKeyboardLayout();
    setupMouseControlLayout();
    establishVirtualKeyboardSingleSignalConnections();
    establishVirtualMouseSignalConnections();

    QTimer::singleShot(0, this, SLOT(setButtonFontSizes()));
}

/*bool VirtualKeyboardMouseWidget::is_numlock_activated()
{
#if defined(WITH_X11)
    Display *dpy = XOpenDisplay(X11Extras::getInstance()->getEnvVariable("DISPLAY"));
    XKeyboardState x;
    XGetKeyboardControl(dpy, &x);
    XCloseDisplay(dpy);
    return x.led_mask & 2;
#endif

    return false;
}

bool VirtualKeyboardMouseWidget::isLaptop()
{
    int secs, pct;

    if (SDL_GetPowerInfo(&secs, &pct) == SDL_POWERSTATE_UNKNOWN) return false;
    else return true;
}*/

void VirtualKeyboardMouseWidget::setupVirtualKeyboardLayout()
{
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(keyboardTab);
    QVBoxLayout *tempMainKeyLayout = setupMainKeyboardLayout();
    QVBoxLayout *tempAuxKeyLayout = new QVBoxLayout();
    QVBoxLayout *tempNumKeyPadLayout = new QVBoxLayout();

    if (m_isNumKeypad)
    {
        tempNumKeyPadLayout = setupKeyboardNumPadLayout();
    } else
    {
        QPushButton *othersKeysButton = createOtherKeysMenu();

        tempNumKeyPadLayout->addWidget(noneButton);
        tempNumKeyPadLayout->addWidget(othersKeysButton);
        tempNumKeyPadLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    }

    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->addLayout(tempMainKeyLayout);

    if (m_isNumKeypad)
    {
        tempAuxKeyLayout = setupAuxKeyboardLayout();
        tempHBoxLayout->addLayout(tempAuxKeyLayout);
    } else
    {
        delete tempAuxKeyLayout;
    }

    tempHBoxLayout->addLayout(tempNumKeyPadLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

QVBoxLayout *VirtualKeyboardMouseWidget::setupMainKeyboardLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);

    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(0);

    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();
    if (!m_isNumKeypad)
        finalVBoxLayout->setSpacing(0);

    tempHBoxLayout->addWidget(createNewKey("Escape"));

    if (m_isNumKeypad)
        tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));

    tempHBoxLayout->addWidget(createNewKey("F1"));
    tempHBoxLayout->addWidget(createNewKey("F2"));
    tempHBoxLayout->addWidget(createNewKey("F3"));
    tempHBoxLayout->addWidget(createNewKey("F4"));

    if (m_isNumKeypad)
        tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));

    tempHBoxLayout->addWidget(createNewKey("F5"));
    tempHBoxLayout->addWidget(createNewKey("F6"));
    tempHBoxLayout->addWidget(createNewKey("F7"));
    tempHBoxLayout->addWidget(createNewKey("F8"));

    if (m_isNumKeypad)
        tempHBoxLayout->addSpacerItem(new QSpacerItem(70, 10, QSizePolicy::Expanding));

    tempHBoxLayout->addWidget(createNewKey("F9"));
    tempHBoxLayout->addWidget(createNewKey("F10"));
    tempHBoxLayout->addWidget(createNewKey("F11"));
    tempHBoxLayout->addWidget(createNewKey("F12"));

    if (!m_isNumKeypad)
    {
        tempHBoxLayout->addWidget(createNewKey("Print"));
        tempHBoxLayout->addWidget(createNewKey("Pause"));
        tempHBoxLayout->addWidget(createNewKey("Delete"));
    }

    finalVBoxLayout->addLayout(tempHBoxLayout);

    if (m_isNumKeypad)
        finalVBoxLayout->addSpacerItem(new QSpacerItem(20, 35, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("grave"));

    for (int i = 1; i <= 9; i++)
    {
        tempHBoxLayout->addWidget(createNewKey(QString::number(i)));
    }

    tempHBoxLayout->addWidget(createNewKey("0"));
    tempHBoxLayout->addWidget(createNewKey("minus"));
    tempHBoxLayout->addWidget(createNewKey("equal"));
    tempHBoxLayout->addWidget(createNewKey("BackSpace"));

    if (!m_isNumKeypad)
    {
        tempHBoxLayout->addWidget(createNewKey("Home"));
    }

    tempVBoxLayout->addLayout(tempHBoxLayout);

    QVBoxLayout *tempMiddleVLayout = new QVBoxLayout();
    tempMiddleVLayout->setSpacing(0);

    QHBoxLayout *tempMiddleHLayout = new QHBoxLayout();
    tempHBoxLayout = new QHBoxLayout();

    tempHBoxLayout->addWidget(createNewKey("Tab"));
    if (m_isNumKeypad)
        tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 30, QSizePolicy::Fixed));
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

    if ((QLocale::system().language() != QLocale::French) && (QLocale::system().language() != QLocale::German))
    {
        tempHBoxLayout->addWidget(createNewKey("backslash"));
    }

    if (!m_isNumKeypad)
    {
        tempHBoxLayout->addWidget(createNewKey("Prior"));
        tempVBoxLayout->addLayout(tempHBoxLayout);
    } else
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

    if ((QLocale::system().language() == QLocale::French) || (QLocale::system().language() == QLocale::German))
    {
        tempHBoxLayout->addWidget(createNewKey("asterisk"));
    }

    if (m_isNumKeypad)
    {
        tempMiddleVLayout->addLayout(tempHBoxLayout);
        tempMiddleHLayout->addLayout(tempMiddleVLayout);
        tempMiddleHLayout->setSpacing(0);
        tempMiddleHLayout->addWidget(createNewKey("Return"));

        tempVBoxLayout->addLayout(tempMiddleHLayout);
    } else
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

    if (!m_isNumKeypad)
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

    VirtualKeyPushButton *altR = createNewKey("Alt_R");

    if (altR->getKeycode() <= 0)
    {
        tempHBoxLayout->addWidget(createNewKey("ISO_Level3_Shift"));
        delete altR;
    } else
    {
        tempHBoxLayout->addWidget(altR);
    }

    tempHBoxLayout->addWidget(createNewKey("Menu"));
    tempHBoxLayout->addWidget(createNewKey("Control_R"));

    if (!m_isNumKeypad)
    {
        tempHBoxLayout->addWidget(createNewKey("Left"));
        tempHBoxLayout->addWidget(createNewKey("Down"));
        tempHBoxLayout->addWidget(createNewKey("Right"));
    }

    tempVBoxLayout->addLayout(tempHBoxLayout);

    if (m_isNumKeypad)
    {
        tempVBoxLayout->setStretch(0, 1);
        tempVBoxLayout->setStretch(1, 2);
        tempVBoxLayout->setStretch(2, 1);
        tempVBoxLayout->setStretch(3, 1);
    }

    finalVBoxLayout->addLayout(tempVBoxLayout);

    if (m_isNumKeypad)
    {
        finalVBoxLayout->setStretch(0, 1);
        finalVBoxLayout->setStretch(1, 0);
        finalVBoxLayout->setStretch(2, 2);
    }

    return finalVBoxLayout;
}

QVBoxLayout *VirtualKeyboardMouseWidget::setupAuxKeyboardLayout()
{
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

QVBoxLayout *VirtualKeyboardMouseWidget::setupKeyboardNumPadLayout()
{
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
    tempGridLayout->addWidget(createNewKey("KP_Delete"), 2, 3, 1, 1);

    tempHBoxLayout->addLayout(tempGridLayout);
    tempHBoxLayout->addWidget(createNewKey("KP_Enter"));

    tempVBoxLayout->addLayout(tempHBoxLayout);
    finalVBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->setStretchFactor(tempVBoxLayout, 8);

    return finalVBoxLayout;
}

void VirtualKeyboardMouseWidget::setupMouseControlLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(mouseTab);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    VirtualMousePushButton *pushButton =
        new VirtualMousePushButton(tr("Left", "Mouse"), JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    pushButton =
        new VirtualMousePushButton(tr("Up", "Mouse"), JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    QHBoxLayout *tempInnerHBoxLayout = new QHBoxLayout();
    pushButton = new VirtualMousePushButton(tr("Left Button", "Mouse"), 1, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton(tr("Middle Button", "Mouse"), 2, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton(tr("Right Button", "Mouse"), 3, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addLayout(tempInnerHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton = new VirtualMousePushButton(tr("Wheel Up", "Mouse"), 4, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 1, 2, 1, 1);
    pushButton = new VirtualMousePushButton(tr("Wheel Left", "Mouse"), 6, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 1, 1, 1);
    pushButton = new VirtualMousePushButton(tr("Wheel Right", "Mouse"), 7, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 3, 1, 1);
    pushButton = new VirtualMousePushButton(tr("Wheel Down", "Mouse"), 5, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 3, 2, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton =
        new VirtualMousePushButton(tr("Down", "Mouse"), JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
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
    pushButton =
        new VirtualMousePushButton(tr("Right", "Mouse"), JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(20);

#ifdef Q_OS_WIN
    pushButton = new VirtualMousePushButton(tr("Button 4", "Mouse"), 8, JoyButtonSlot::JoyMouseButton, this);
#else
    pushButton = new VirtualMousePushButton(tr("Mouse 8", "Mouse"), 8, JoyButtonSlot::JoyMouseButton, this);
#endif

    pushButton->setMinimumHeight(40);
    tempVBoxLayout->addWidget(pushButton);

#ifdef Q_OS_WIN
    pushButton = new VirtualMousePushButton(tr("Button 5", "Mouse"), 9, JoyButtonSlot::JoyMouseButton, this);
#else
    pushButton = new VirtualMousePushButton(tr("Mouse 9", "Mouse"), 9, JoyButtonSlot::JoyMouseButton, this);
#endif

    pushButton->setMinimumHeight(40);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(20);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    mouseSettingsPushButton = new QPushButton(tr("Mouse Settings"), this);
    mouseSettingsPushButton->setIcon(PadderCommon::loadIcon("input-mouse", ":/images/actions/edit_select.png"));

    tempVBoxLayout->addWidget(mouseSettingsPushButton);

    mouseSettingsPushButton->setEnabled(false);

    tempHBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

VirtualKeyPushButton *VirtualKeyboardMouseWidget::createNewKey(QString xcodestring)
{
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
    } else if (xcodestring == "Tab")
    {
        width = 43;
    } else if (xcodestring == "Shift_L")
    {
        width = 77;
    } else if (xcodestring == "Shift_R")
    {
        if (!m_isNumKeypad)
            width = 59;
        else
            width = 95;
    } else if (xcodestring == "Caps_Lock")
    {
        width = 59;
    } else if (xcodestring == "Return")
    {
        width = 64;
        if (m_isNumKeypad)
            height = 64;
        pushButton->setMaximumWidth(100);
    } else if (xcodestring == "BackSpace")
    {
        width = 68;
    } else if (topRowKeys.contains(xcodestring))
    {
        width = 30;
        height = 32;
        pushButton->setMaximumSize(100, 100);
    } else if ((xcodestring == "Print") || (xcodestring == "Scroll_Lock") || (xcodestring == "Pause"))
    {
        width = 32;
        height = 32;
        pushButton->setMaximumSize(100, 100);
        font1.setPointSize(6);
    } else if ((xcodestring == "KP_Add") || (xcodestring == "KP_Enter"))
    {
        height = 56;
        width = 32;
        font1.setPointSize(5);
    } else if (xcodestring == "Num_Lock")
    {
        width = 32;
        height = 32;
        font1.setPointSize(5);
    } else if (xcodestring.startsWith("KP_"))
    {
        width = 32;
    } else if (xcodestring == "backslash")
    {
        if (m_isNumKeypad)
            width = 32;
        else
            width = 43;
    } else if ((xcodestring == "Down") || (xcodestring == "Left") || (xcodestring == "Right"))
    {
        if (!m_isNumKeypad)
        {
            QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
            pushButton->setSizePolicy(sizePolicy2);
            pushButton->setFixedWidth(58);
        }
    } else if ((xcodestring == "Control_L") || (xcodestring == "Super_L") || (xcodestring == "Alt_L") ||
               (xcodestring == "Alt_R") || (xcodestring == "ISO_Level3_Shift") || (xcodestring == "Menu") ||
               (xcodestring == "Control_R"))
    {
        if (!m_isNumKeypad)
            width = 32;
        else
            width = 41;
    } else if ((xcodestring.startsWith("F") && (xcodestring.length() > 1)) || (xcodestring == "Print") ||
               (xcodestring == "Escape") || (xcodestring == "Pause") || (xcodestring == "Delete"))
    {
        if (!m_isNumKeypad)
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

QPushButton *VirtualKeyboardMouseWidget::createNoneKey()
{
    QPushButton *pushButton = new QPushButton(tr("NONE"), this);
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
    if ((currentQuickDialog == nullptr) && !withoutQuickSetDialog)
    {
        currentQuickDialog = new QuickSetDialog(joystick, helper, "setAssignedSlot", keycode, alias, 0,
                                                JoyButtonSlot::JoyKeyboard, true, true, this);
        currentQuickDialog->show();
        connect(currentQuickDialog, &QuickSetDialog::finished, this, &VirtualKeyboardMouseWidget::nullifyDialogPointer);
    } else if (withoutQuickSetDialog)
    {
        ButtonEditDialog::getInstance()->invokeMethodLastBtn(lastPressedBtn, helper, "setAssignedSlot", keycode, alias, 0,
                                                             JoyButtonSlot::JoyKeyboard, true, true, Qt::QueuedConnection,
                                                             Qt::QueuedConnection, Qt::QueuedConnection);
        ButtonEditDialog::getInstance()->refreshForLastBtn();
    }
}

void VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection(int keycode, int alias)
{
    emit selectionMade(keycode, alias);
}

void VirtualKeyboardMouseWidget::processSingleMouseSelection(JoyButtonSlot *tempslot)
{
    if ((currentQuickDialog == nullptr) && !withoutQuickSetDialog)
    {
        currentQuickDialog = new QuickSetDialog(joystick, helper, "setAssignedSlot", tempslot->getSlotCode(), -1, -1,
                                                tempslot->getSlotMode(), true, true, this);
        currentQuickDialog->show();
        connect(currentQuickDialog, &QuickSetDialog::finished, this, &VirtualKeyboardMouseWidget::nullifyDialogPointer);
    } else if (withoutQuickSetDialog)
    {
        ButtonEditDialog::getInstance()->invokeMethodLastBtn(
            lastPressedBtn, helper, "setAssignedSlot", tempslot->getSlotCode(), -1, -1, tempslot->getSlotMode(), true, true,
            Qt::QueuedConnection, Qt::QueuedConnection, Qt::QueuedConnection);
        ButtonEditDialog::getInstance()->refreshForLastBtn();
    }
}

void VirtualKeyboardMouseWidget::processAdvancedMouseSelection(JoyButtonSlot *tempslot) { emit selectionMade(tempslot); }

void VirtualKeyboardMouseWidget::populateTopRowKeys()
{
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

void VirtualKeyboardMouseWidget::addFButtonToOthers(int qt_keycode, QString keycode_text)
{
    if (keysymToKeyString(AntKeyMapper::getInstance()->returnVirtualKey(qt_keycode)) != tr("[NO KEY]"))
    {
        QAction *tempAction = new QAction(keycode_text, otherKeysMenu);
        tempAction->setData(AntKeyMapper::getInstance()->returnVirtualKey(qt_keycode));
        otherKeysMenu->addAction(tempAction);
    }
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections()
{
    QList<VirtualKeyPushButton *> newlist = keyboardTab->findChildren<VirtualKeyPushButton *>();
    QListIterator<VirtualKeyPushButton *> iter(newlist);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, &VirtualKeyPushButton::keycodeObtained, nullptr, nullptr);
        connect(keybutton, &VirtualKeyPushButton::keycodeObtained, this,
                &VirtualKeyboardMouseWidget::processSingleKeyboardSelection);
    }

    QListIterator<QAction *> iterActions(otherKeysMenu->actions());

    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, &QAction::triggered, nullptr, nullptr);

        connect(temp, &QAction::triggered, this, [this, temp](bool checked) { otherKeysActionSingle(temp, checked); });
    }

    disconnect(noneButton, &QPushButton::clicked, nullptr, nullptr);
    connect(noneButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::clearButtonSlotsFinish);
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections()
{
    QList<VirtualKeyPushButton *> newlist = keyboardTab->findChildren<VirtualKeyPushButton *>();
    QListIterator<VirtualKeyPushButton *> iter(newlist);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, &VirtualKeyPushButton::keycodeObtained, nullptr, nullptr);
        connect(keybutton, &VirtualKeyPushButton::keycodeObtained, this,
                &VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection);
    }

    QListIterator<QAction *> iterActions(otherKeysMenu->actions());

    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, &QAction::triggered, nullptr, nullptr);

        connect(temp, &QAction::triggered, this, [this, temp](bool checked) { otherKeysActionAdvanced(temp, checked); });
    }

    disconnect(noneButton, &QPushButton::clicked, nullptr, nullptr);
    connect(noneButton, &QPushButton::clicked, this, &VirtualKeyboardMouseWidget::clearButtonSlots);
}

void VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections()
{
    QList<VirtualMousePushButton *> newlist = mouseTab->findChildren<VirtualMousePushButton *>();
    QListIterator<VirtualMousePushButton *> iter(newlist);

    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, nullptr, nullptr);
        connect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, this,
                &VirtualKeyboardMouseWidget::processSingleMouseSelection);
    }
}

void VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections()
{
    QList<VirtualMousePushButton *> newlist = mouseTab->findChildren<VirtualMousePushButton *>();
    QListIterator<VirtualMousePushButton *> iter(newlist);

    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, nullptr, nullptr);
        connect(mousebutton, &VirtualMousePushButton::mouseSlotCreated, this,
                &VirtualKeyboardMouseWidget::processAdvancedMouseSelection);
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlots()
{
    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
            QMetaObject::invokeMethod(ButtonEditDialog::getInstance()->getLastJoyButton(), "clearSlotsEventReset",
                                      Qt::BlockingQueuedConnection);

        emit selectionCleared();

        lastPressedBtn->buildActiveZoneSummaryString();

    } else
    {
        QMessageBox::information(this, tr("Last button"),
                                 tr("Slots for button couldn't be cleared, because there was not any set button from "
                                    "keyboard for gamepad. Map at least one button from keyboard to gamepad"));
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlotsFinish()
{
    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
            QMetaObject::invokeMethod(ButtonEditDialog::getInstance()->getLastJoyButton(), "clearSlotsEventReset",
                                      Qt::BlockingQueuedConnection);

        emit selectionFinished();

        lastPressedBtn->buildActiveZoneSummaryString();

    } else
    {
        QMessageBox::information(this, tr("Last button"),
                                 tr("Slots for button couldn't be cleared, because there was not any set button from "
                                    "keyboard for gamepad. Map at least one button from keyboard to gamepad"));
    }
}

bool VirtualKeyboardMouseWidget::isKeyboardTabVisible() { return this->keyboardTab->isVisible(); }

void VirtualKeyboardMouseWidget::openMouseSettingsDialog()
{
    mouseSettingsPushButton->setEnabled(false);

    // TODO instead of buttons, get pointer to static getInstance from ButtonEditDialog for last pressed button, and then
    // getting button from public method
    if (ButtonEditDialog::getInstance() != nullptr)
    {
        if (ButtonEditDialog::getInstance()->getLastJoyButton() != nullptr)
        {
            MouseButtonSettingsDialog *dialog =
                new MouseButtonSettingsDialog(ButtonEditDialog::getInstance()->getLastJoyButton(), this);
            dialog->show();
            QDialog *parent = qobject_cast<QDialog *>(this->parentWidget()); // static_cast
            connect(parent, &QDialog::finished, dialog, &MouseButtonSettingsDialog::close);
            connect(dialog, &MouseButtonSettingsDialog::finished, this,
                    &VirtualKeyboardMouseWidget::enableMouseSettingButton);
        } else
        {
            QMessageBox::information(this, QObject::tr("Last button"),
                                     QObject::tr("There isn't a last button pressed from gamepad in data. Did you set at "
                                                 "least one button from gamepad for keyboard?"));
        }
    }
}

void VirtualKeyboardMouseWidget::enableMouseSettingButton() { mouseSettingsPushButton->setEnabled(true); }

void VirtualKeyboardMouseWidget::disableMouseSettingButton() { mouseSettingsPushButton->setEnabled(false); }

void VirtualKeyboardMouseWidget::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);
    setButtonFontSizes();
}

// Dynamically change font size of list of push button according to the
// size of the buttons.
void VirtualKeyboardMouseWidget::setButtonFontSizes()
{
    QList<VirtualKeyPushButton *> buttonList = this->findChildren<VirtualKeyPushButton *>();
    QListIterator<VirtualKeyPushButton *> iter(buttonList);

    while (iter.hasNext())
    {
        VirtualKeyPushButton *temp = iter.next();
        QFont tempFont(temp->font());
        tempFont.setPointSize(temp->calculateFontSize());
        temp->setFont(tempFont);
    }
}

QPushButton *VirtualKeyboardMouseWidget::createOtherKeysMenu()
{
    QPushButton *otherKeysPushbutton = new QPushButton(tr("Others"), this);
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
    tempAction = new QAction(tr("Applications"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Menu);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);
#endif

    tempAction = new QAction(tr("Browser Back"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Back);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Favorites"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Favorites);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Forward"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Forward);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Home"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_HomePage);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Refresh"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Refresh);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Search"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Search);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Browser Stop"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Stop);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Calc"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Launch1);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Email"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_LaunchMail);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    addFButtonToOthers(Qt::Key_F13, tr("F13"));
    addFButtonToOthers(Qt::Key_F14, tr("F14"));
    addFButtonToOthers(Qt::Key_F15, tr("F15"));
    addFButtonToOthers(Qt::Key_F16, tr("F16"));
    addFButtonToOthers(Qt::Key_F17, tr("F17"));
    addFButtonToOthers(Qt::Key_F18, tr("F18"));
    addFButtonToOthers(Qt::Key_F19, tr("F19"));
    addFButtonToOthers(Qt::Key_F20, tr("F20"));
    addFButtonToOthers(Qt::Key_F21, tr("F21"));
    addFButtonToOthers(Qt::Key_F22, tr("F22"));
    addFButtonToOthers(Qt::Key_F23, tr("F23"));
    addFButtonToOthers(Qt::Key_F24, tr("F24"));

    tempAction = new QAction(tr("Media"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_LaunchMedia);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Media Next"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaNext);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Media Play"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaPlay);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Media Previous"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaPrevious);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Media Stop"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_MediaStop);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Search"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Search);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Volume Down"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeDown);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Volume Mute"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeMute);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    tempAction = new QAction(tr("Volume Up"), otherKeysMenu);
    temp = AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_VolumeUp);
    tempAction->setData(temp);
    otherKeysMenu->addAction(tempAction);

    otherKeysPushbutton->setMenu(otherKeysMenu);
    return otherKeysPushbutton;
}

void VirtualKeyboardMouseWidget::otherKeysActionSingle(QAction *tempAction, bool triggered)
{
    Q_UNUSED(triggered);

    int virtualkey = tempAction->data().toInt();
    processSingleKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}

void VirtualKeyboardMouseWidget::otherKeysActionAdvanced(QAction *tempAction, bool triggered)
{
    Q_UNUSED(triggered);

    int virtualkey = tempAction->data().toInt();
    processAdvancedKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}

void VirtualKeyboardMouseWidget::nullifyDialogPointer()
{
    if (currentQuickDialog != nullptr)
    {
        ButtonEditDialog::getInstance()->setUpLastJoyButton(currentQuickDialog->getLastPressedButton());
        ButtonEditDialog::getInstance()->refreshForLastBtn();
        enableMouseSettingButton();
        currentQuickDialog = nullptr;
        emit buttonDialogClosed();
    }
}

InputDevice *VirtualKeyboardMouseWidget::getJoystick() const { return joystick; }

ButtonEditDialogHelper *VirtualKeyboardMouseWidget::getHelper() const { return helper; }

QWidget *VirtualKeyboardMouseWidget::getKeyboardTab() const { return keyboardTab; }

QWidget *VirtualKeyboardMouseWidget::getMouseTab() const { return mouseTab; }

QPushButton *VirtualKeyboardMouseWidget::getNoneButton() const { return noneButton; }

QPushButton *VirtualKeyboardMouseWidget::getMouseSettingsPushButton() const { return mouseSettingsPushButton; }

QMenu *VirtualKeyboardMouseWidget::getOtherKeysMenu() const { return otherKeysMenu; }

QuickSetDialog *VirtualKeyboardMouseWidget::getCurrentQuickDialog() const { return currentQuickDialog; }
