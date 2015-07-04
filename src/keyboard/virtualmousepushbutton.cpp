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

#include "virtualmousepushbutton.h"

VirtualMousePushButton::VirtualMousePushButton(QString displayText, int code, JoyButtonSlot::JoySlotInputAction mode, QWidget *parent) :
    QPushButton(parent)
{
    if (mode == JoyButtonSlot::JoyMouseButton || mode == JoyButtonSlot::JoyMouseMovement)
    {
        this->setText(displayText);

        if (mode == JoyButtonSlot::JoyMouseMovement)
        {
            switch (code)
            {
                case JoyButtonSlot::MouseUp:
                case JoyButtonSlot::MouseDown:
                case JoyButtonSlot::MouseLeft:
                case JoyButtonSlot::MouseRight:
                {
                    this->code = code;
                    break;
                }
                default:
                {
                    this->code = 0;
                    break;
                }
            }
        }
        else
        {
            this->code = code;
        }
        this->mode = mode;
    }
    else
    {
        this->setText(tr("INVALID"));
        this->code = 0;
        this->mode = JoyButtonSlot::JoyMouseButton;
    }

    connect(this, SIGNAL(clicked()), this, SLOT(createTempSlot()));
}

unsigned int VirtualMousePushButton::getMouseCode()
{
    return code;
}

JoyButtonSlot::JoySlotInputAction VirtualMousePushButton::getMouseMode()
{
    return mode;
}

void VirtualMousePushButton::createTempSlot()
{
    JoyButtonSlot *tempslot = new JoyButtonSlot(this->code, this->mode, this);
    emit mouseSlotCreated(tempslot);
}
