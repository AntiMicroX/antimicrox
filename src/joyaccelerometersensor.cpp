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

#include "joyaccelerometersensor.h"

JoyAccelerometerSensor::JoyAccelerometerSensor(double rate, int originset, SetJoystick *parent_set, QObject *parent)
    : JoySensor(ACCELEROMETER, originset, parent_set, parent)
{
}

JoyAccelerometerSensor::~JoyAccelerometerSensor() {}

/**
 * @brief Get the value for the corresponding X axis.
 * @return X axis value in m/s^2
 */
float JoyAccelerometerSensor::getXCoordinate() const { return m_current_value[0]; }

/**
 * @brief Get the value for the corresponding Y axis.
 * @return Y axis value in m/s^2
 */
float JoyAccelerometerSensor::getYCoordinate() const { return m_current_value[1]; }

/**
 * @brief Get the value for the corresponding Z axis.
 * @return Z axis value in m/s^2
 */
float JoyAccelerometerSensor::getZCoordinate() const { return m_current_value[2]; }

/**
 * @brief Get the translated sensor type name
 * @returns Translated sensor type name
 */
QString JoyAccelerometerSensor::sensorTypeName() const { return tr("Accelerometer"); }
