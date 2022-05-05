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

#include "joygyroscopesensor.h"
#include "joybuttontypes/joygyroscopebutton.h"

JoyGyroscopeSensor::JoyGyroscopeSensor(int originset, SetJoystick *parent_set, QObject *parent)
    : JoySensor(GYROSCOPE, originset, parent_set, parent)
{
    populateButtons();
}

JoyGyroscopeSensor::~JoyGyroscopeSensor() {}

/**
 * @brief Get the value for the corresponding X axis.
 * @return X axis value in °/s
 */
float JoyGyroscopeSensor::getXCoordinate() const { return radToDeg(m_current_value[0]); }

/**
 * @brief Get the value for the corresponding Y axis.
 * @return Y axis value in °/s
 */
float JoyGyroscopeSensor::getYCoordinate() const { return radToDeg(m_current_value[1]); }

/**
 * @brief Get the value for the corresponding Z axis.
 * @return Z axis value in °/s
 */
float JoyGyroscopeSensor::getZCoordinate() const { return radToDeg(m_current_value[2]); }

/**
 * @brief Get the translated sensor type name
 * @returns Translated sensor type name
 */
QString JoyGyroscopeSensor::sensorTypeName() const { return tr("Gyroscope"); }

/**
 * @brief Initializes the JoySensorButton objects for this sensor.
 */
void JoyGyroscopeSensor::populateButtons()
{
    JoySensorButton *button = nullptr;
    button = new JoyGyroscopeButton(this, SENSOR_LEFT, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_LEFT, button);

    button = new JoyGyroscopeButton(this, SENSOR_RIGHT, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_RIGHT, button);

    button = new JoyGyroscopeButton(this, SENSOR_UP, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_UP, button);

    button = new JoyGyroscopeButton(this, SENSOR_DOWN, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_DOWN, button);

    button = new JoyGyroscopeButton(this, SENSOR_FWD, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_FWD, button);

    button = new JoyGyroscopeButton(this, SENSOR_BWD, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_BWD, button);
}
