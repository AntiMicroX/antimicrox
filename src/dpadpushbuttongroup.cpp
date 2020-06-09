/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "dpadpushbuttongroup.h"

#include "messagehandler.h"
#include "joydpad.h"
#include "joydpadbuttonwidget.h"
#include "dpadpushbutton.h"
#include "buttoneditdialog.h"
#include "dpadeditdialog.h"
#include "inputdevice.h"

#include <QHash>
#include <QWidget>
#include <QDebug>

DPadPushButtonGroup::DPadPushButtonGroup(JoyDPad *dpad, bool keypadUnlocked, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->dpad = dpad;
    this->displayNames = displayNames;
    this->keypadUnlocked = keypadUnlocked;

    generateButtons();
    changeButtonLayout();

    connect(dpad, &JoyDPad::joyModeChanged, this, &DPadPushButtonGroup::changeButtonLayout);
}

void DPadPushButtonGroup::generateButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

    upLeftButton = generateBtnForGrid(buttons, 9, 0, 0);
    upButton = generateBtnForGrid(buttons, 1, 0, 1);
    upRightButton = generateBtnForGrid(buttons, 3, 0, 2);
    leftButton = generateBtnForGrid(buttons, 8, 1, 0);

    dpadWidget = new DPadPushButton(dpad, displayNames, parentWidget());
    dpadWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games_config_options"),
                                         QIcon(":/icons/hicolor/16x16/actions/games_config_options.png")));
    connect(dpadWidget, &DPadPushButton::clicked, this, &DPadPushButtonGroup::showDPadDialog);
    addWidget(dpadWidget, 1, 1);

    rightButton = generateBtnForGrid(buttons, 2, 1, 2);
    downLeftButton = generateBtnForGrid(buttons, 12, 2, 0);
    downButton = generateBtnForGrid(buttons, 4, 2, 1);
    downRightButton = generateBtnForGrid(buttons, 6, 2, 2);
}

JoyDPadButtonWidget* DPadPushButtonGroup::generateBtnForGrid(QHash<int, JoyDPadButton*> *buttons, int dpadDirection, int cellRow, int cellCol)
{
    JoyDPadButton *button = buttons->value(static_cast<JoyDPadButton::JoyDPadDirections>(dpadDirection));
    JoyDPadButtonWidget *pushbutton = new JoyDPadButtonWidget(button, displayNames, parentWidget());

    connect(pushbutton, &JoyDPadButtonWidget::clicked, this, [this, pushbutton] {
        openDPadButtonDialog(pushbutton);
    });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoyDPadButton::slotsChanged, this, &DPadPushButtonGroup::propogateSlotsChanged);
    addWidget(pushbutton, cellRow, cellCol);

    return pushbutton;
}

void DPadPushButtonGroup::changeButtonLayout()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((dpad->getJoyMode() == JoyDPad::StandardMode) ||
        (dpad->getJoyMode() == JoyDPad::EightWayMode) ||
        (dpad->getJoyMode() == JoyDPad::FourWayCardinal))
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

    if ((dpad->getJoyMode() == JoyDPad::EightWayMode) ||
        (dpad->getJoyMode() == JoyDPad::FourWayDiagonal))
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

void DPadPushButtonGroup::propogateSlotsChanged()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit buttonSlotChanged();
}

JoyDPad* DPadPushButtonGroup::getDPad() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return dpad;
}

void DPadPushButtonGroup::openDPadButtonDialog(JoyButtonWidget* buttonWidget)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, dpad->getParentSet()->getInputDevice(), keypadUnlocked, parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::showDPadDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    DPadEditDialog *dialog = new DPadEditDialog(dpad, parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::toggleNameDisplay()
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

    dpadWidget->toggleNameDisplay();
}
