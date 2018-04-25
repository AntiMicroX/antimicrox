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

#include "dpadeditdialoghelper.h"
#include "joybuttonslot.h"

#include <QHashIterator>
#include <QDebug>

DPadEditDialogHelper::DPadEditDialogHelper(JoyDPad *dpad, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_ASSERT(dpad);

    this->dpad = dpad;
}

void DPadEditDialogHelper::setPendingSlots(QHash<JoyDPadButton::JoyDPadDirections, JoyButtonSlot *> *tempSlots)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    pendingSlots.clear();

    QHashIterator<JoyDPadButton::JoyDPadDirections, JoyButtonSlot*> iter(*tempSlots);
    while (iter.hasNext())
    {
        iter.next();

        JoyButtonSlot *slot = iter.value();
        JoyDPadButton::JoyDPadDirections tempDir = iter.key();
        pendingSlots.insert(tempDir, slot);
    }
}

void DPadEditDialogHelper::clearPendingSlots()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    pendingSlots.clear();
}

void DPadEditDialogHelper::setFromPendingSlots()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (!getPendingSlots().isEmpty())
    {
        QHashIterator<JoyDPadButton::JoyDPadDirections, JoyButtonSlot*> iter(getPendingSlots());
        while (iter.hasNext())
        {
            iter.next();

            JoyButtonSlot *slot = iter.value();
            if (slot)
            {
                JoyDPadButton::JoyDPadDirections tempDir = iter.key();
                JoyDPadButton *button = dpad->getJoyButton(tempDir);
                button->clearSlotsEventReset(false);
                button->setAssignedSlot(slot->getSlotCode(), slot->getSlotCodeAlias(),
                                        slot->getSlotMode());
                slot->deleteLater();
            }
        }
    }
}

void DPadEditDialogHelper::clearButtonsSlotsEventReset()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QHash<int, JoyDPadButton*> *buttons = dpad->getButtons();
    QHashIterator<int, JoyDPadButton*> iter(*buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->clearSlotsEventReset();
    }
}

void DPadEditDialogHelper::updateJoyDPadDelay(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int temp = value * 10;
    if (dpad->getDPadDelay() != temp)
    {
        dpad->setDPadDelay(temp);
    }
}


QHash<JoyDPadButton::JoyDPadDirections, JoyButtonSlot*> const& DPadEditDialogHelper::getPendingSlots() {

    return pendingSlots;
}
