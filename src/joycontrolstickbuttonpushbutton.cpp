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

#include "joycontrolstickbuttonpushbutton.h"

#include "messagehandler.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"
#include "joybuttoncontextmenu.h"
#include "joycontrolstick.h"

#include <QMenu>
#include <QWidget>
#include <QDebug>

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(JoyControlStickButton *button, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->button = button;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoyControlStickButtonPushButton::customContextMenuRequested, this, &JoyControlStickButtonPushButton::showContextMenu);
    connect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button, &JoyControlStickButton::activeZoneChanged, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button->getStick()->getModifierButton(), &JoyControlStickModifierButton::activeZoneChanged,
            this, &JoyControlStickButtonPushButton::refreshLabel);
}

JoyControlStickButton* JoyControlStickButtonPushButton::getButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return button;
}

void JoyControlStickButtonPushButton::setButton(JoyControlStickButton *button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disableFlashes();
    if (this->button != nullptr)
    {
        disconnect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
        disconnect(this->button, &JoyControlStickButton::activeZoneChanged, this, &JoyControlStickButtonPushButton::refreshLabel);
    }

    this->button = button;
    refreshLabel();
    enableFlashes();

    connect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button, &JoyControlStickButton::activeZoneChanged, this, &JoyControlStickButtonPushButton::refreshLabel, Qt::QueuedConnection);
}


void JoyControlStickButtonPushButton::disableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (button != nullptr)
    {
        disconnect(button, &JoyControlStickButton::clicked, this, &JoyControlStickButtonPushButton::flash);
        disconnect(button, &JoyControlStickButton::released, this, &JoyControlStickButtonPushButton::unflash);
    }
    this->unflash();
}

void JoyControlStickButtonPushButton::enableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (button != nullptr)
    {
        connect(button, &JoyControlStickButton::clicked, this, &JoyControlStickButtonPushButton::flash, Qt::QueuedConnection);
        connect(button, &JoyControlStickButton::released, this, &JoyControlStickButtonPushButton::unflash, Qt::QueuedConnection);
    }
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyControlStickButtonPushButton::generateLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    if (button != nullptr)
    {
        if (!button->getActionName().isEmpty() && ifDisplayNames())
        {
            qDebug() << "Action name was not empty";

            temp = button->getActionName().replace("&", "&&");

        }
        else
        {
            qDebug() << "Action name was empty";

            temp = button->getCalculatedActiveZoneSummary().replace("&", "&&");
        }
    }

    qDebug() << "Here is name of action for pushed stick button: " << temp;

    return temp;
}

void JoyControlStickButtonPushButton::showContextMenu(const QPoint &point)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyControlStickButtonPushButton::tryFlash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (button->getButtonState())
    {
        flash();
    }
}
