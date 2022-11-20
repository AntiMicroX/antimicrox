/* antimicrox Gamepad to KB+M event mapper
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

#include "buttoneditdialoghelper.h"

#include "joybuttontypes/joybutton.h"

#include <QDebug>

ButtonEditDialogHelper::ButtonEditDialogHelper(JoyButton *button, QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(button);

    this->button = button;
}

ButtonEditDialogHelper::ButtonEditDialogHelper(QObject *parent)
    : QObject(parent)
{
}

void ButtonEditDialogHelper::setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode)
{
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(code, mode);
}

void ButtonEditDialogHelper::setAssignedSlot(int code, int alias, JoyButtonSlot::JoySlotInputAction mode)
{
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(code, alias, mode);
}

void ButtonEditDialogHelper::setAssignedSlot(int code, int alias, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    button->clearSlotsEventReset(false);
    button->setAssignedSlot(code, alias, index, mode);
}

void ButtonEditDialogHelper::setUseTurbo(bool useTurbo) { button->setUseTurbo(useTurbo); }

void ButtonEditDialogHelper::setThisButton(JoyButton *btn)
{
    Q_ASSERT(btn);

    button = btn;
}
