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
#pragma once

#include "joysensordirection.h"

#include <QHash>
#include <QObject>

class JoyButtonSlot;
class JoySensor;

/**
 * @brief Some helper methods which run in the IO thread and are called
 *   from the GUI thread.
 */
class JoySensorIoThreadHelper : public QObject
{
    Q_OBJECT

  public:
    explicit JoySensorIoThreadHelper(JoySensor *sensor, QObject *parent = nullptr);
    void setPendingSlots(QHash<JoySensorDirection, JoyButtonSlot *> *tempSlots);
    void clearPendingSlots();

  public slots:
    void setFromPendingSlots();
    void clearButtonsSlotsEventReset();

  private:
    JoySensor *m_sensor;
    QHash<JoySensorDirection, JoyButtonSlot *> m_pending_slots;
};
