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
#include <QFont>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QListIterator>
#include <QLocale>

#include "virtualkeyboardmousewidget.h"
#include <mousedialog/mousebuttonsettingsdialog.h>
#include <event.h>
#include <antkeymapper.h>

QHash<QString, QString> VirtualKeyboardMouseWidget::topRowKeys = QHash<QString, QString> ();

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(JoyButton *button, QWidget *parent) :
    QTabWidget(parent)
{
    this->button = button;
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

    connect(mouseSettingsPushButton, SIGNAL(clicked()), this, SLOT(openMouseSettingsDialog()));
}

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(QWidget *parent) :
    QTabWidget(parent)
{
    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    noneButton = createNoneKey();

    populateTopRowKeys();

    this->addTab(keyboardTab, tr("Keyboard"));
    this->addTab(mouseTab, tr("Mouse"));

    this->setTabPosition(QTabWidget::South);

    QTimer::singleShot(0, this, SLOT(setButtonFontSizes()));
}

void VirtualKeyboardMouseWidget::setupVirtualKeyboardLayout()
{
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(keyboardTab);

    QVBoxLayout *tempMainKeyLayout = setupMainKeyboardLayout();
    QVBoxLayout *tempAuxKeyLayout = setupAuxKeyboardLayout();
    QVBoxLayout *tempNumKeyPadLayout = setupKeyboardNumPadLayout();
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();

    tempHBoxLayout->addLayout(tempMainKeyLayout);
    tempHBoxLayout->addLayout(tempAuxKeyLayout);
    tempHBoxLayout->addLayout(tempNumKeyPadLayout);

    finalVBoxLayout->addLayout(tempHBoxLayout);
}

QVBoxLayout *VirtualKeyboardMouseWidget::setupMainKeyboardLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();

    tempHBoxLayout->addWidget(createNewKey("Escape"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F1"));
    tempHBoxLayout->addWidget(createNewKey("F2"));
    tempHBoxLayout->addWidget(createNewKey("F3"));
    tempHBoxLayout->addWidget(createNewKey("F4"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F5"));
    tempHBoxLayout->addWidget(createNewKey("F6"));
    tempHBoxLayout->addWidget(createNewKey("F7"));
    tempHBoxLayout->addWidget(createNewKey("F8"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));

    tempHBoxLayout->addWidget(createNewKey("F9"));
    tempHBoxLayout->addWidget(createNewKey("F10"));
    tempHBoxLayout->addWidget(createNewKey("F11"));
    tempHBoxLayout->addWidget(createNewKey("F12"));
    finalVBoxLayout->addLayout(tempHBoxLayout);

    finalVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

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
    tempVBoxLayout->addLayout(tempHBoxLayout);

    QVBoxLayout *tempMiddleVLayout = new QVBoxLayout();
    QHBoxLayout *tempMiddleHLayout = new QHBoxLayout();
    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Tab"));
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
    if (QLocale::system().language() != QLocale::French &&
        QLocale::system().language() != QLocale::German)
    {
        tempHBoxLayout->addWidget(createNewKey("backslash"));
    }
    tempMiddleVLayout->addLayout(tempHBoxLayout);

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
    if (QLocale::system().language() == QLocale::French ||
        QLocale::system().language() == QLocale::German)
    {
        tempHBoxLayout->addWidget(createNewKey("asterisk"));
    }
    tempMiddleVLayout->addLayout(tempHBoxLayout);
    tempMiddleHLayout->addLayout(tempMiddleVLayout);
    tempMiddleHLayout->addWidget(createNewKey("Return"));
    tempVBoxLayout->addLayout(tempMiddleHLayout);

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
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Control_L"));
    tempHBoxLayout->addWidget(createNewKey("Super_L"));
    tempHBoxLayout->addWidget(createNewKey("Alt_L"));
    tempHBoxLayout->addWidget(createNewKey("space"));
    tempHBoxLayout->addWidget(createNewKey("Alt_R"));
    tempHBoxLayout->addWidget(createNewKey("Menu"));
    tempHBoxLayout->addWidget(createNewKey("Control_R"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempVBoxLayout->setStretch(0, 1);
    tempVBoxLayout->setStretch(1, 2);
    tempVBoxLayout->setStretch(2, 1);
    tempVBoxLayout->setStretch(3, 1);

    finalVBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->setStretch(0, 1);
    finalVBoxLayout->setStretch(1, 0);
    finalVBoxLayout->setStretch(2, 2);
    return finalVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupAuxKeyboardLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();

    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Print"));
    tempHBoxLayout->addWidget(createNewKey("Scroll_Lock"));
    tempHBoxLayout->addWidget(createNewKey("Pause"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("Insert"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Home"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Prior"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("Delete"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("End"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Next"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout = new QGridLayout();
    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("Up"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Left"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Down"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Right"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    return tempVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupKeyboardNumPadLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();

    QPushButton *othersKeysButton = createOtherKeysMenu();

    //tempHBoxLayout->addWidget(othersKeysButton);
    //tempHBoxLayout->addWidget(noneButton);

    //finalVBoxLayout->addLayout(tempHBoxLayout);
    finalVBoxLayout->addWidget(noneButton);
    finalVBoxLayout->addWidget(othersKeysButton);
    finalVBoxLayout->setStretchFactor(noneButton, 1);
    finalVBoxLayout->setStretchFactor(othersKeysButton, 1);
    finalVBoxLayout->addSpacerItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

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
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(mouseTab);
    VirtualMousePushButton *pushButton = 0;
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    pushButton = new VirtualMousePushButton(tr("Left", "Mouse"), JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    pushButton = new VirtualMousePushButton(tr("Up", "Mouse"), JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
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

    pushButton = new VirtualMousePushButton(tr("Down", "Mouse"), JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
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
    pushButton = new VirtualMousePushButton(tr("Right", "Mouse"), JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
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
    mouseSettingsPushButton->setIcon(QIcon::fromTheme(QString::fromUtf8("edit-select")));
    tempVBoxLayout->addWidget(mouseSettingsPushButton);

    tempHBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

VirtualKeyPushButton* VirtualKeyboardMouseWidget::createNewKey(QString xcodestring)
{
    int width = 30;
    int height = 30;
    QFont font1;
    font1.setPointSize(8);
    font1.setBold(true);

    VirtualKeyPushButton *pushButton = new VirtualKeyPushButton(button, xcodestring, this);

    if (xcodestring == "space")
    {
        width = 100;
    }
    else if (xcodestring == "Tab")
    {
        width = 40;
    }
    else if (xcodestring == "Shift_L" || xcodestring == "Shift_R")
    {
        width = 84;
    }
    else if (xcodestring == "Control_L")
    {
        width = 70;
    }
    else if (xcodestring == "Return")
    {
        width = 60;
        height = 60;
        pushButton->setMaximumWidth(100);
    }
    else if (xcodestring == "BackSpace")
    {
        width = 72;
    }
    else if (topRowKeys.contains(xcodestring))
    {
        width = 30;
        height = 36;
        pushButton->setMaximumSize(100, 100);
    }
    else if (xcodestring == "Print" || xcodestring == "Scroll_Lock" || xcodestring == "Pause")
    {
        width = 40;
        height = 36;
        pushButton->setMaximumSize(100, 100);
        font1.setPointSize(6);
    }
    else if (xcodestring == "KP_Add" || xcodestring == "KP_Enter")
    {
        width = 34;
        font1.setPointSize(6);
    }
    else if (xcodestring == "Num_Lock")
    {
        font1.setPointSize(6);
    }
    else if (xcodestring.startsWith("KP_"))
    {
        width = 36;
        height = 32;
    }

    pushButton->setObjectName(xcodestring);
    pushButton->setMinimumSize(width, height);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setFont(font1);

    return pushButton;
}

QPushButton* VirtualKeyboardMouseWidget::createNoneKey()
{
    QPushButton *pushButton = new QPushButton(tr("NONE"), this);
    pushButton->setMinimumSize(0, 25);
    //pushButton->setMaximumHeight(100);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    pushButton->setSizePolicy(sizePolicy);
    QFont font1;
    font1.setBold(true);
    pushButton->setFont(font1);

    return pushButton;
}

void VirtualKeyboardMouseWidget::processSingleKeyboardSelection(int keycode, unsigned int alias)
{
    QMetaObject::invokeMethod(button, "clearSlotsEventReset");
    //button->clearSlotsEventReset();
    QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, keycode),
                              Q_ARG(unsigned int, alias),
                              Q_ARG(int, 0),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyKeyboard));
    //button->setAssignedSlot(keycode, alias);

    emit selectionFinished();
}

void VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection(int keycode, unsigned int alias)
{
    emit selectionMade(keycode, alias);
}

void VirtualKeyboardMouseWidget::processSingleMouseSelection(JoyButtonSlot *tempslot)
{
    QMetaObject::invokeMethod(button, "clearSlotsEventReset");
    //button->clearSlotsEventReset();
    QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, tempslot->getSlotCode()),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, tempslot->getSlotMode()));
    //button->setAssignedSlot(tempslot->getSlotCode(), tempslot->getSlotMode());
    emit selectionFinished();
}

void VirtualKeyboardMouseWidget::processAdvancedMouseSelection(JoyButtonSlot *tempslot)
{
    emit selectionMade(tempslot);
}

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

void VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections()
{
    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, SIGNAL(keycodeObtained(int, unsigned int)), 0, 0);
        connect(keybutton, SIGNAL(keycodeObtained(int, unsigned int)), this, SLOT(processSingleKeyboardSelection(int, unsigned int)));
    }

    QListIterator<QAction*> iterActions(otherKeysMenu->actions());
    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, SIGNAL(triggered(bool)), 0, 0);
        connect(temp, SIGNAL(triggered(bool)), this, SLOT(otherKeysActionSingle(bool)));
    }

    disconnect(noneButton, SIGNAL(clicked()), 0, 0);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(clearButtonSlotsFinish()));
    //qDebug() << "COUNT: " << newlist.count();
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections()
{
    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, SIGNAL(keycodeObtained(int, unsigned int)), 0, 0);
        connect(keybutton, SIGNAL(keycodeObtained(int, unsigned int)), this, SLOT(processAdvancedKeyboardSelection(int, unsigned int)));
    }

    QListIterator<QAction*> iterActions(otherKeysMenu->actions());
    while (iterActions.hasNext())
    {
        QAction *temp = iterActions.next();
        disconnect(temp, SIGNAL(triggered(bool)), 0, 0);
        connect(temp, SIGNAL(triggered(bool)), this, SLOT(otherKeysActionAdvanced(bool)));
    }

    disconnect(noneButton, SIGNAL(clicked()), 0, 0);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(clearButtonSlots()));
}

void VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections()
{
    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), 0, 0);
        connect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), this, SLOT(processSingleMouseSelection(JoyButtonSlot*)));
    }
}

void VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections()
{
    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), 0, 0);
        connect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), this, SLOT(processAdvancedMouseSelection(JoyButtonSlot*)));
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlots()
{
    QMetaObject::invokeMethod(button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
    //button->clearSlotsEventReset();
    emit selectionCleared();
}

void VirtualKeyboardMouseWidget::clearButtonSlotsFinish()
{
    QMetaObject::invokeMethod(button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
    //button->clearSlotsEventReset();
    emit selectionFinished();
}

bool VirtualKeyboardMouseWidget::isKeyboardTabVisible()
{
    return this->keyboardTab->isVisible();
}

void VirtualKeyboardMouseWidget::openMouseSettingsDialog()
{
    mouseSettingsPushButton->setEnabled(false);

    MouseButtonSettingsDialog *dialog = new MouseButtonSettingsDialog(this->button, this);
    dialog->show();
    QDialog *parent = static_cast<QDialog*>(this->parentWidget());
    connect(parent, SIGNAL(finished(int)), dialog, SLOT(close()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(enableMouseSettingButton()));
}

void VirtualKeyboardMouseWidget::enableMouseSettingButton()
{
    mouseSettingsPushButton->setEnabled(true);
}

void VirtualKeyboardMouseWidget::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);
    setButtonFontSizes();
}

// Dynamically change font size of list of push button according to the
// size of the buttons.
void VirtualKeyboardMouseWidget::setButtonFontSizes()
{
    //int tempWidgetFontSize = 20;
    QList<VirtualKeyPushButton*> buttonList = this->findChildren<VirtualKeyPushButton*>();
    QListIterator<VirtualKeyPushButton*> iter(buttonList);
    while (iter.hasNext())
    {
        VirtualKeyPushButton *temp = iter.next();
        //widgetSizeMan = qMin(temp->calculateFontSize(), tempWidgetFontSize);
        QFont tempFont(temp->font());
        tempFont.setPointSize(temp->calculateFontSize());
        temp->setFont(tempFont);
        //temp->update();
    }

    /*iter.toFront();

    while (iter.hasNext())
    {
        VirtualKeyPushButton *temp = iter.next();
        QFont tempFont(temp->font());
        tempFont.setPointSize(widgetSizeMan);
        temp->setFont(tempFont);
    }
    */
}

QPushButton* VirtualKeyboardMouseWidget::createOtherKeysMenu()
{
    QPushButton *otherKeysPushbutton = new QPushButton("Others", this);
    otherKeysPushbutton->setMinimumSize(0, 25);
    //fuckMotherFuck->setMaximumHeight(100);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    otherKeysPushbutton->setSizePolicy(sizePolicy);
    QFont font1;
    font1.setBold(true);
    otherKeysPushbutton->setFont(font1);

    otherKeysMenu = new QMenu(this);

    QAction *tempAction = 0;
    unsigned int temp = 0;

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

void VirtualKeyboardMouseWidget::otherKeysActionSingle(bool triggered)
{
    Q_UNUSED(triggered);

    QAction *tempAction = static_cast<QAction*>(sender());
    unsigned int virtualkey = tempAction->data().toInt();
    processSingleKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}

void VirtualKeyboardMouseWidget::otherKeysActionAdvanced(bool triggered)
{
    Q_UNUSED(triggered);

    QAction *tempAction = static_cast<QAction*>(sender());
    unsigned int virtualkey = tempAction->data().toInt();
    processAdvancedKeyboardSelection(virtualkey, AntKeyMapper::getInstance()->returnQtKey(virtualkey));
}
