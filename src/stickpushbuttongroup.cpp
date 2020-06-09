/* antimicroX Gamepad to KB+M event mapper
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



StickPushButtonGroup::StickPushButtonGroup(JoyControlStick *stick, bool keypadUnlocked, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->keypadUnlocked = keypadUnlocked;
    this->stick = stick;
    this->displayNames = displayNames;

    generateButtons();
    changeButtonLayout();

    connect(stick, &JoyControlStick::joyModeChanged, this, &StickPushButtonGroup::changeButtonLayout);
}

void StickPushButtonGroup::generateButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickButtonPushButton *pushbutton = nullptr;

    upLeftButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickLeftUp, 0, 0);
    upButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickUp, 0, 1);
    upRightButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickRightUp, 0, 2);
    leftButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickLeft, 1, 0);

    stickWidget = new JoyControlStickPushButton(stick, displayNames, parentWidget());
    stickWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games_config_options"),
                                          QIcon(":/icons/hicolor/16x16/actions/games_config_options.png")));

    connect(stickWidget, &JoyControlStickPushButton::clicked, this, &StickPushButtonGroup::showStickDialog);

    addWidget(stickWidget, 1, 1);

    rightButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickRight, 1, 2);
    downLeftButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickLeftDown, 2, 0);
    downButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickDown, 2, 1);
    downRightButton = generateBtnToGrid(pushbutton, stick, JoyControlStick::StickRightDown, 2, 2);
}

JoyControlStickButtonPushButton* StickPushButtonGroup::generateBtnToGrid(JoyControlStickButtonPushButton *pushbutton, JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections stickValue, int gridRow, int gridCol)
{
    JoyControlStickButton* button = stick->getButtons()->value(stickValue);
    pushbutton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());

    connect(pushbutton, &JoyControlStickButtonPushButton::clicked, this, [this, pushbutton] {
        openStickButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyControlStickButton::slotsChanged, this, &StickPushButtonGroup::propogateSlotsChanged);

    addWidget(pushbutton, gridRow, gridCol);
    return pushbutton;
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

    ButtonEditDialog *dialog = new ButtonEditDialog(pushbutton->getButton(), stick->getParentSet()->getInputDevice(), keypadUnlocked, parentWidget());
    dialog->show();
}

void StickPushButtonGroup::showStickDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickEditDialog *dialog = new JoyControlStickEditDialog(stick, keypadUnlocked, parentWidget());
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
