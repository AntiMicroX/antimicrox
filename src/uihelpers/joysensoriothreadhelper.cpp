/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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

#include "joysensoriothreadhelper.h"

#include "joybuttonslot.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joysensor.h"

#include <QDebug>

JoySensorIoThreadHelper::JoySensorIoThreadHelper(JoySensor *sensor, QObject *parent)
    : QObject(parent)
    , m_sensor(sensor)
{
    Q_ASSERT(m_sensor);
}

/**
 * @brief Sets pending slot values
 *  The values are applied to the buttons by calling setFromPendingSlots.
 * @param[in] tempSlots QHash of new pending slot values.
 *  Buttons that are absent from the hash will be left unchanged.
 */
void JoySensorIoThreadHelper::setPendingSlots(QHash<JoySensorDirection, JoyButtonSlot *> *tempSlots)
{
    m_pending_slots.clear();
    for (auto iter = tempSlots->cbegin(); iter != tempSlots->cend(); ++iter)
    {
        JoyButtonSlot *slot = iter.value();
        JoySensorDirection tempDir = iter.key();
        m_pending_slots.insert(tempDir, slot);
    }
}

/**
 * @brief Clears pending slot values
 */
void JoySensorIoThreadHelper::clearPendingSlots() { m_pending_slots.clear(); }

/**
 * @brief Applies pending slot values to the buttons
 *  Should be called via QMetaObject::invokeMethod
 */
void JoySensorIoThreadHelper::setFromPendingSlots()
{
    if (!m_pending_slots.isEmpty())
    {
        for (auto iter = m_pending_slots.cbegin(); iter != m_pending_slots.cend(); ++iter)
        {
            JoyButtonSlot *slot = iter.value();
            if (slot)
            {
                JoySensorDirection tempDir = iter.key();
                JoySensorButton *button = m_sensor->getDirectionButton(tempDir);
                if (button)
                {
                    button->clearSlotsEventReset(false);
                    button->setAssignedSlot(slot->getSlotCode(), slot->getSlotCodeAlias(), slot->getSlotMode());
                }
                slot->deleteLater();
            }
        }
    }
}

/**
 * @brief Calls JoyButton::clearSlotEventReset on all JoyButtons of the underlying sensor.
 *  Should be called via QMetaObject::invokeMethod
 */
void JoySensorIoThreadHelper::clearButtonsSlotsEventReset()
{
    QHash<JoySensorDirection, JoySensorButton *> *buttons = m_sensor->getButtons();
    for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
    {
        JoySensorButton *button = iter.value();
        if (button)
            button->clearSlotsEventReset();
    }
}
