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

#include <QMetaType>

/**
 * @brief A bitfield style enum which encodes all possible three dimensional
 *  sensor directions. This allows triggering the mapped buttons by simply
 *  checking the bits for the six basic directions.
 */
enum JoySensorDirection
{
    SENSOR_CENTERED = 0,
    SENSOR_LEFT = (1 << 0),
    SENSOR_RIGHT = (1 << 1),
    SENSOR_UP = (1 << 2),
    SENSOR_DOWN = (1 << 3),
    SENSOR_FWD = (1 << 4),
    SENSOR_BWD = (1 << 5),

    SENSOR_LEFT_UP = SENSOR_LEFT | SENSOR_UP,
    SENSOR_LEFT_DOWN = SENSOR_LEFT | SENSOR_DOWN,
    SENSOR_LEFT_FWD = SENSOR_LEFT | SENSOR_FWD,
    SENSOR_LEFT_BWD = SENSOR_LEFT | SENSOR_BWD,
    SENSOR_RIGHT_UP = SENSOR_RIGHT | SENSOR_UP,
    SENSOR_RIGHT_DOWN = SENSOR_RIGHT | SENSOR_DOWN,
    SENSOR_RIGHT_FWD = SENSOR_RIGHT | SENSOR_FWD,
    SENSOR_RIGHT_BWD = SENSOR_RIGHT | SENSOR_BWD,
    SENSOR_UP_FWD = SENSOR_UP | SENSOR_FWD,
    SENSOR_UP_BWD = SENSOR_UP | SENSOR_BWD,
    SENSOR_DOWN_FWD = SENSOR_DOWN | SENSOR_FWD,
    SENSOR_DOWN_BWD = SENSOR_DOWN | SENSOR_BWD,
    SENSOR_LEFT_UP_FWD = SENSOR_LEFT | SENSOR_UP | SENSOR_FWD,
    SENSOR_LEFT_UP_BWD = SENSOR_LEFT | SENSOR_UP | SENSOR_BWD,
    SENSOR_LEFT_DOWN_FWD = SENSOR_LEFT | SENSOR_DOWN | SENSOR_FWD,
    SENSOR_LEFT_DOWN_BWD = SENSOR_LEFT | SENSOR_DOWN | SENSOR_BWD,
    SENSOR_RIGHT_UP_FWD = SENSOR_RIGHT | SENSOR_UP | SENSOR_FWD,
    SENSOR_RIGHT_UP_BWD = SENSOR_RIGHT | SENSOR_UP | SENSOR_BWD,
    SENSOR_RIGHT_DOWN_FWD = SENSOR_RIGHT | SENSOR_DOWN | SENSOR_FWD,
    SENSOR_RIGHT_DOWN_BWD = SENSOR_RIGHT | SENSOR_DOWN | SENSOR_BWD
};

Q_DECLARE_METATYPE(JoySensorDirection)
