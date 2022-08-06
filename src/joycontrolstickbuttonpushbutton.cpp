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

#include "joycontrolstickbuttonpushbutton.h"

#include "joybuttoncontextmenu.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"
#include "joycontrolstick.h"
#include "logger.h"

#include <QMenu>
#include <QWidget>

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(JoyControlStickButton *button, bool displayNames,
                                                                 QWidget *parent)
    : FlashButtonWidget(displayNames, parent)
{
    this->button = button;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoyControlStickButtonPushButton::customContextMenuRequested, this,
            &JoyControlStickButtonPushButton::showContextMenu);
    connect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button, &JoyControlStickButton::activeZoneChanged, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button->getStick()->getModifierButton(), &JoyControlStickModifierButton::activeZoneChanged, this,
            &JoyControlStickButtonPushButton::refreshLabel);
}

JoyControlStickButton *JoyControlStickButtonPushButton::getButton() { return button; }

void JoyControlStickButtonPushButton::setButton(JoyControlStickButton *button)
{
    disableFlashes();
    if (this->button != nullptr)
    {
        disconnect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
        disconnect(this->button, &JoyControlStickButton::activeZoneChanged, this,
                   &JoyControlStickButtonPushButton::refreshLabel);
    }

    this->button = button;
    refreshLabel();
    enableFlashes();

    connect(button, &JoyControlStickButton::propertyUpdated, this, &JoyControlStickButtonPushButton::refreshLabel);
    connect(button, &JoyControlStickButton::activeZoneChanged, this, &JoyControlStickButtonPushButton::refreshLabel,
            Qt::QueuedConnection);
}

void JoyControlStickButtonPushButton::disableFlashes()
{
    if (button != nullptr)
    {
        disconnect(button, &JoyControlStickButton::clicked, this, &JoyControlStickButtonPushButton::flash);
        disconnect(button, &JoyControlStickButton::released, this, &JoyControlStickButtonPushButton::unflash);
    }
    this->unflash();
}

void JoyControlStickButtonPushButton::enableFlashes()
{
    if (button != nullptr)
    {
        connect(button, &JoyControlStickButton::clicked, this, &JoyControlStickButtonPushButton::flash,
                Qt::QueuedConnection);
        connect(button, &JoyControlStickButton::released, this, &JoyControlStickButtonPushButton::unflash,
                Qt::QueuedConnection);
    }
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyControlStickButtonPushButton::generateLabel()
{
    QString temp = QString();
    if (button != nullptr)
    {
        if (!button->getActionName().isEmpty() && ifDisplayNames())
        {
            temp = button->getActionName().replace("&", "&&");

            DEBUG() << "Name of action for pushed stick button: " << temp << " (Action name was not empty)";

        } else
        {
            temp = button->getCalculatedActiveZoneSummary().replace("&", "&&");
            DEBUG() << "Name of action for pushed stick button: " << temp << " (Action name was empty)";
        }
    }

    return temp;
}

void JoyControlStickButtonPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyControlStickButtonPushButton::tryFlash()
{
    if (button->getButtonState())
    {
        flash();
    }
}
