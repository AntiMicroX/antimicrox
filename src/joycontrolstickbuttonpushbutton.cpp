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

#include "joycontrolstickbuttonpushbutton.h"
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->button = button;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    //connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));
    connect(button->getStick()->getModifierButton(), SIGNAL(activeZoneChanged()),
            this, SLOT(refreshLabel()));
}

JoyControlStickButton* JoyControlStickButtonPushButton::getButton()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return button;
}

void JoyControlStickButtonPushButton::setButton(JoyControlStickButton *button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disableFlashes();
    if (this->button != nullptr)
    {
        //disconnect(this->button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
        disconnect(button, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
        disconnect(this->button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));
    }

    this->button = button;
    refreshLabel();
    enableFlashes();
    //connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()), Qt::QueuedConnection);
}


void JoyControlStickButtonPushButton::disableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (button != nullptr)
    {
        disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
        disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    }
    this->unflash();
}

void JoyControlStickButtonPushButton::enableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (button != nullptr)
    {
        connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
        connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
    }
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyControlStickButtonPushButton::generateLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (button != nullptr)
    {
        if (!button->getActionName().isEmpty() && displayNames)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyControlStickButtonPushButton::tryFlash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (button->getButtonState())
    {
        flash();
    }
}
