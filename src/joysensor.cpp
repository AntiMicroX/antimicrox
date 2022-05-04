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

#define _USE_MATH_DEFINES

#include "joysensor.h"

#include <cmath>

JoySensor::JoySensor(JoySensorType type, int originset, SetJoystick *parent_set, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_originset(originset)
    , m_parent_set(parent_set)
{
}

JoySensor::~JoySensor() {}

void JoySensor::queuePendingEvent(float *values, bool ignoresets) {}

/**
 * @brief Returns the sensor type
 */
JoySensorType JoySensor::getType() const { return m_type; }

bool JoySensor::inDeadZone(float *values) const { return false; }

/**
 * @brief Utility function which converts a given value from radians to degree.
 */
double JoySensor::radToDeg(double value) { return value * 180 / M_PI; }

/**
 * @brief Utility function which converts a given value from degree to radians.
 */
double JoySensor::degToRad(double value) { return value * M_PI / 180; }

bool JoySensor::isDefault() const { return false; }
