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

#include "joygyroscopesensor.h"
#include "globalvariables.h"
#include "joybuttontypes/joygyroscopebutton.h"

#include <cmath>

JoyGyroscopeSensor::JoyGyroscopeSensor(int originset, SetJoystick *parent_set, QObject *parent)
    : JoySensor(GYROSCOPE, originset, parent_set, parent)
{
    reset();
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
 * @brief Reads the calibration values of the sensor
 * @param[out] offsetX Offset value for X axis
 * @param[out] offsetY Offset value for Y axis
 * @param[out] offsetZ Offset value for Z axis
 */
void JoyGyroscopeSensor::getCalibration(double *offsetX, double *offsetY, double *offsetZ) const
{
    *offsetX = m_calibration_value[0];
    *offsetY = m_calibration_value[1];
    *offsetZ = m_calibration_value[2];
}

/**
 * @brief Sets the sensor calibration values and sets the calibration flag.
 * @param[in] offsetX Offset value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] offsetZ Offset value for Z axis
 */
void JoyGyroscopeSensor::setCalibration(double offsetX, double offsetY, double offsetZ)
{
    m_calibration_value[0] = offsetX;
    m_calibration_value[1] = offsetY;
    m_calibration_value[2] = offsetZ;
    m_calibrated = true;
}

/**
 * @brief Resets internal variables back to default
 */
void JoyGyroscopeSensor::reset()
{
    JoySensor::reset();
    m_max_zone = degToRad(GlobalVariables::JoySensor::GYRO_MAX);
}

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

/**
 * @brief Applies calibration to queued input values
 */
void JoyGyroscopeSensor::applyCalibration()
{
    m_pending_value[0] -= m_calibration_value[0];
    m_pending_value[1] -= m_calibration_value[1];
    m_pending_value[2] -= m_calibration_value[2];
}

/**
 * @brief Find the direction zone of the current sensor position.
 *
 * First, the sensor axis values are normalized so they are on the unit sphere.
 * Then, the unit sphere is divided into direction zones with the following algorithm:
 *   - Mark a spherical layer around the X axis at +/- the diagonal zone angle
 *     divided by two (called "range" in the code)
 *     Then generate two more spherical layers by rotating the
 *     first layer around the Y and Z axes.
 * Check if a point is within each layer by comparing the absolute value
 * of each coordinate against the "range".
 * If a point is in only one layer, it is in the diagonal zone between two axes.
 * If a point is in two layers, it is in the orthogonal zone of one axis.
 * If a point is in three or zero zones, it is diagonal to all three axes.
 * There are two cases here because the spherical layers overlap if the diagonal
 * angle is larger then 45 degree.
 *
 * @returns JoySensorDirection bitfield for the current direction zone.
 */
JoySensorDirection JoyGyroscopeSensor::calculateSensorDirection()
{
    double distance = calculateDistance();
    if (distance < m_dead_zone)
        return SENSOR_CENTERED;

    double range = sin(M_PI / 4 - m_diagonal_range / 2);
    double normPitch = m_current_value[0] / distance;
    double normRoll = m_current_value[1] / distance;
    double normYaw = m_current_value[2] / distance;

    bool inPitch = abs(normPitch) < range;
    bool inRoll = abs(normRoll) < range;
    bool inYaw = abs(normYaw) < range;

    if (inPitch && !inRoll && !inYaw)
    {
        if (normRoll > 0)
        {
            if (normYaw > 0)
                return SENSOR_RIGHT_FWD;
            else
                return SENSOR_LEFT_FWD;
        } else
        {
            if (normYaw > 0)
                return SENSOR_RIGHT_BWD;
            else
                return SENSOR_LEFT_BWD;
        }
    } else if (!inPitch && inRoll && !inYaw)
    {
        if (normPitch > 0)
        {
            if (normYaw > 0)
                return SENSOR_RIGHT_UP;
            else
                return SENSOR_LEFT_UP;
        } else
        {
            if (normYaw > 0)
                return SENSOR_RIGHT_DOWN;
            else
                return SENSOR_LEFT_DOWN;
        }
    } else if (!inPitch && !inRoll && inYaw)
    {
        if (normPitch > 0)
        {
            if (normRoll > 0)
                return SENSOR_UP_FWD;
            else
                return SENSOR_UP_BWD;
        } else
        {
            if (normRoll > 0)
                return SENSOR_DOWN_FWD;
            else
                return SENSOR_DOWN_BWD;
        }
    } else if (inPitch && inRoll && !inYaw)
    {
        if (normYaw > 0)
            return SENSOR_RIGHT;
        else
            return SENSOR_LEFT;
    } else if (inPitch && !inRoll && inYaw)
    {
        if (normRoll > 0)
            return SENSOR_FWD;
        else
            return SENSOR_BWD;
    } else if (!inPitch && inRoll && inYaw)
    {
        if (normPitch > 0)
            return SENSOR_UP;
        else
            return SENSOR_DOWN;
    } else // in all or in none
    {
        if (normPitch > 0)
        {
            if (normRoll > 0)
            {
                if (normYaw > 0)
                    return SENSOR_RIGHT_UP_FWD;
                else
                    return SENSOR_LEFT_UP_FWD;
            } else
            {
                if (normYaw > 0)
                    return SENSOR_RIGHT_UP_BWD;
                else
                    return SENSOR_LEFT_UP_BWD;
            }
        } else
        {
            if (normRoll > 0)
            {
                if (normYaw > 0)
                    return SENSOR_RIGHT_DOWN_FWD;
                else
                    return SENSOR_LEFT_DOWN_FWD;
            } else
            {
                if (normYaw > 0)
                    return SENSOR_RIGHT_DOWN_BWD;
                else
                    return SENSOR_LEFT_DOWN_BWD;
            }
        }
    }
}
