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

#include "joyaccelerometerbutton.h"
#include "joysensordirection.h"

JoyAccelerometerButton::JoyAccelerometerButton(JoySensor *sensor, int sdl_button_index, int originset,
                                               SetJoystick *parentSet, QObject *parent)
    : JoySensorButton(sensor, sdl_button_index, originset, parentSet, parent)
{
}

/**
 * @brief Get translated accelerometer direction name of this button
 * @returns Translated direction name
 */
QString JoyAccelerometerButton::getDirectionName() const
{
    QString label = QString();

    switch (m_index_sdl)
    {
    case JoySensorDirection::SENSOR_UP:
        label.append(tr("Up"));
        break;

    case JoySensorDirection::SENSOR_DOWN:
        label.append(tr("Down"));
        break;

    case JoySensorDirection::SENSOR_LEFT:
        label.append(tr("Left"));
        break;

    case JoySensorDirection::SENSOR_RIGHT:
        label.append(tr("Right"));
        break;

    case JoySensorDirection::SENSOR_BWD:
        label.append(tr("Shock"));
        break;

    default:
        WARN() << "unknown accelerometer direction";
        break;
    }

    return label;
}
