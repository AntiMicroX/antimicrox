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

#include "joyaxiscontextmenuhelper.h"

JoyAxisContextMenuHelper::JoyAxisContextMenuHelper(JoyAxis *axis, QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(axis);

    this->axis = axis;
}

void JoyAxisContextMenuHelper::setNAssignedSlot(int code, unsigned int alias,
                                               JoyButtonSlot::JoySlotInputAction mode)
{
    JoyButton *button = axis->getNAxisButton();
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(code, alias, mode);
}

void JoyAxisContextMenuHelper::setPAssignedSlot(int code, unsigned int alias,
                                               JoyButtonSlot::JoySlotInputAction mode)
{
    JoyButton *button = axis->getPAxisButton();
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(code, alias, mode);
}

void JoyAxisContextMenuHelper::clearAndResetAxisButtons()
{
    JoyAxisButton *nbutton = axis->getNAxisButton();
    JoyAxisButton *pbutton = axis->getPAxisButton();
    nbutton->clearSlotsEventReset();
    pbutton->clearSlotsEventReset();
}
