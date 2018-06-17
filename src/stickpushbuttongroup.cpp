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

#include "stickpushbuttongroup.h"

#include "messagehandler.h"
#include "buttoneditdialog.h"
#include "joycontrolstickeditdialog.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joycontrolstickpushbutton.h"
#include "joycontrolstickbuttonpushbutton.h"
#include "inputdevice.h"

#include <QHash>
#include <QWidget>
#include <QDebug>

StickPushButtonGroup::StickPushButtonGroup(JoyControlStick *stick, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->stick = stick;
    this->displayNames = displayNames;

    generateButtons();
    changeButtonLayout();

    connect(stick, &JoyControlStick::joyModeChanged, this, &StickPushButtonGroup::changeButtonLayout);
}

void StickPushButtonGroup::generateButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();

    JoyControlStickButton *button = nullptr;
    JoyControlStickButtonPushButton *pushbutton = nullptr;

    button = stickButtons->value(JoyControlStick::StickLeftUp);
    upLeftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upLeftButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 0, 0);

    button = stickButtons->value(JoyControlStick::StickUp);
    upButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 0, 1);

    button = stickButtons->value(JoyControlStick::StickRightUp);
    upRightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upRightButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 0, 2);

    button = stickButtons->value(JoyControlStick::StickLeft);
    leftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = leftButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 1, 0);

    stickWidget = new JoyControlStickPushButton(stick, displayNames, parentWidget());
    stickWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
    connect(stickWidget, &JoyControlStickPushButton::clicked, this, &StickPushButtonGroup::showStickDialog);

    addWidget(stickWidget, 1, 1);

    button = stickButtons->value(JoyControlStick::StickRight);
    rightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = rightButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 1, 2);

    button = stickButtons->value(JoyControlStick::StickLeftDown);
    downLeftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downLeftButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();

    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 2, 0);

    button = stickButtons->value(JoyControlStick::StickDown);
    downButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 2, 1);

    button = stickButtons->value(JoyControlStick::StickRightDown);
    downRightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downRightButton;
    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, 2, 2);
}

void StickPushButtonGroup::changeButtonLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((stick->getJoyMode() == JoyControlStick::StandardMode) ||
        (stick->getJoyMode() == JoyControlStick::EightWayMode) ||
        (stick->getJoyMode() == JoyControlStick::FourWayCardinal))
    {
        upButton->setVisible(true);
        downButton->setVisible(true);
        leftButton->setVisible(true);
        rightButton->setVisible(true);
    }
    else
    {
        upButton->setVisible(false);
        downButton->setVisible(false);
        leftButton->setVisible(false);
        rightButton->setVisible(false);
    }

    if ((stick->getJoyMode() == JoyControlStick::EightWayMode) ||
        (stick->getJoyMode() == JoyControlStick::FourWayDiagonal))
    {
        upLeftButton->setVisible(true);
        upRightButton->setVisible(true);
        downLeftButton->setVisible(true);
        downRightButton->setVisible(true);
    }
    else
    {
        upLeftButton->setVisible(false);
        upRightButton->setVisible(false);
        downLeftButton->setVisible(false);
        downRightButton->setVisible(false);
    }
}

void StickPushButtonGroup::propogateSlotsChanged()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit buttonSlotChanged();
}

JoyControlStick* StickPushButtonGroup::getStick() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return stick;
}

void StickPushButtonGroup::openStickButtonDialog(JoyControlStickButtonPushButton* pushbutton)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ButtonEditDialog *dialog = new ButtonEditDialog(pushbutton->getButton(), stick->getParentSet()->getInputDevice(), parentWidget());
    dialog->show();
}

void StickPushButtonGroup::showStickDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickEditDialog *dialog = new JoyControlStickEditDialog(stick, parentWidget());
    dialog->show();
}

void StickPushButtonGroup::toggleNameDisplay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    displayNames = !displayNames;

    upButton->toggleNameDisplay();
    downButton->toggleNameDisplay();
    leftButton->toggleNameDisplay();
    rightButton->toggleNameDisplay();

    upLeftButton->toggleNameDisplay();
    upRightButton->toggleNameDisplay();
    downLeftButton->toggleNameDisplay();
    downRightButton->toggleNameDisplay();

    stickWidget->toggleNameDisplay();
}

bool StickPushButtonGroup::ifDisplayNames() const {

    return displayNames;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getUpButton() const {

    return upButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getDownButton() const {

    return downButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getLeftButton() const {

    return leftButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getRightButton() const {

    return rightButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getUpLeftButton() const {

    return upLeftButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getUpRightButton() const {

    return upRightButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getDownLeftButton() const {

    return downLeftButton;
}

JoyControlStickButtonPushButton *StickPushButtonGroup::getDownRightButton() const {

    return downRightButton;
}

JoyControlStickPushButton *StickPushButtonGroup::getStickWidget() const {

    return stickWidget;
}
