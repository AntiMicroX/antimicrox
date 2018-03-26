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

#include "advancebuttondialoghelper.h"
#include "joybutton.h"

#include <QDebug>

AdvanceButtonDialogHelper::AdvanceButtonDialogHelper(JoyButton *button,
                                                     QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_ASSERT(button);

    this->button = button;
}

void AdvanceButtonDialogHelper::insertAssignedSlot(int code, int alias, int index,
                                                    JoyButtonSlot::JoySlotInputAction mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->eventReset();
    button->insertAssignedSlot(code, alias, index, mode);
}

void AdvanceButtonDialogHelper::setAssignedSlot(JoyButtonSlot *otherSlot, int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->eventReset();
    button->setAssignedSlot(otherSlot, index);
}

void AdvanceButtonDialogHelper::setAssignedSlot(int code, int alias, int index,
                                                JoyButtonSlot::JoySlotInputAction mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->eventReset();
    button->setAssignedSlot(code, alias, index, mode);
}

void AdvanceButtonDialogHelper::removeAssignedSlot(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->eventReset();
    button->removeAssignedSlot(index);
}
