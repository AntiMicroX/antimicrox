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

#include "joyaccelerometersensor.h"
#include "globalvariables.h"
#include "joybuttontypes/joyaccelerometerbutton.h"

#include <cmath>

const double JoyAccelerometerSensor::SHOCK_DETECT_THRESHOLD = 20.0;
const double JoyAccelerometerSensor::SHOCK_SUPPRESS_FACTOR = 0.5;
const double JoyAccelerometerSensor::SHOCK_TAU = 0.05;

JoyAccelerometerSensor::JoyAccelerometerSensor(double rate, int originset, SetJoystick *parent_set, QObject *parent)
    : JoySensor(ACCELEROMETER, originset, parent_set, parent)
    , m_shock_filter(SHOCK_TAU, rate)
{
    reset();
    populateButtons();
    m_rate = qFuzzyIsNull(rate) ? PT1Filter::FALLBACK_RATE : rate;
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

/**
 * @brief Reads the calibration values of the sensor
 * @param[out] offsetX Offset angle around the X axis
 * @param[out] offsetY Offset angle around the Y axis
 * @param[out] offsetZ Offset angle around the Z axis
 */
void JoyAccelerometerSensor::getCalibration(double *offsetX, double *offsetY, double *offsetZ) const
{
    *offsetX = m_calibration_value[0];
    *offsetY = m_calibration_value[1];
    *offsetZ = m_calibration_value[2];
}

/**
 * @brief Sets the sensor calibration values and sets the calibration flag.
 * @param[in] offsetX Offset angle around the X axis
 * @param[in] offsetY Offset angle around the Y axis
 * @param[in] offsetZ Offset angle around the Z axis
 *
 * This stores the orientation vector to store the calibration data later and
 * calculates the neutral position rotation matrix from the orientation
 * vector.
 */
void JoyAccelerometerSensor::setCalibration(double offsetX, double offsetY, double offsetZ)
{
    m_calibration_value[0] = offsetX;
    m_calibration_value[1] = offsetY;
    m_calibration_value[2] = offsetZ;

    double rad = sqrt(offsetX * offsetX + offsetY * offsetY + offsetZ * offsetZ);
    double syz = sqrt(offsetY * offsetY + offsetZ * offsetZ);

    m_calibration_matrix[0][0] = syz / rad;
    m_calibration_matrix[0][1] = -offsetX * offsetY / syz / rad;
    m_calibration_matrix[0][2] = -offsetX * offsetZ / syz / rad;
    m_calibration_matrix[1][0] = 0;
    m_calibration_matrix[1][1] = -offsetZ / syz;
    m_calibration_matrix[1][2] = offsetY / syz;
    m_calibration_matrix[2][0] = -offsetX / rad;
    m_calibration_matrix[2][1] = -offsetY / rad;
    m_calibration_matrix[2][2] = -offsetZ / rad;

    m_calibrated = true;
}

/**
 * @brief Resets internal variables back to default
 */
void JoyAccelerometerSensor::reset()
{
    JoySensor::reset();
    m_max_zone = degToRad(GlobalVariables::JoySensor::ACCEL_MAX);

    m_shock_filter.reset();
    m_shock_suppress_count = 0;
}

/**
 * @brief Initializes the JoySensorButton objects for this sensor.
 */
void JoyAccelerometerSensor::populateButtons()
{
    JoySensorButton *button = nullptr;
    button = new JoyAccelerometerButton(this, SENSOR_LEFT, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_LEFT, button);

    button = new JoyAccelerometerButton(this, SENSOR_RIGHT, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_RIGHT, button);

    button = new JoyAccelerometerButton(this, SENSOR_UP, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_UP, button);

    button = new JoyAccelerometerButton(this, SENSOR_DOWN, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_DOWN, button);

    button = new JoyAccelerometerButton(this, SENSOR_BWD, m_originset, getParentSet(), this);
    m_buttons.insert(SENSOR_BWD, button);
}

/**
 * @brief Find the direction zone of the current sensor position.
 *
 * First, the pitch and roll angles on the unit sphere are calculated.
 * Then, the unit sphere is divided into direction zones with the following algorithm:
 *   - Mark a spherical layer around the X axis at +/- the diagonal zone angle
 *     divided by two (called "range" in the code)
 *   - Generate another spherical layers by rotating the first layer around the Y axis.
 *     A third layer is not necessary because there are only two degrees of freedom.
 * Check if a point is within each layer by comparing the absolute values
 * of pitch and roll angles against the "range".
 * If a point is in only one layer, it is in the orthogonal zone of one axis.
 * If a point is in both or no zones, it is diagonal to both axes.
 * There are two cases here because the spherical layers overlap if the diagonal
 * angle is larger then 45 degree.
 *
 * Perform shock detection by taking the first order lag filtered absolute sum of
 * all axes from "joyEvent" and apply a threshold. Discard some samples after
 * the shock is over to avoid spurious pitch/roll events.
 *
 * @returns JoySensorDirection bitfield for the current direction zone.
 */
JoySensorDirection JoyAccelerometerSensor::calculateSensorDirection()
{
    double abs_sum = abs(m_current_value[0]) + abs(m_current_value[1]) + abs(m_current_value[2]);
    if (m_shock_filter.process(abs_sum) > SHOCK_DETECT_THRESHOLD)
    {
        m_shock_suppress_count = m_rate * SHOCK_SUPPRESS_FACTOR;
        return SENSOR_BWD;
    } else if (m_shock_suppress_count != 0)
    {
        --m_shock_suppress_count;
        return SENSOR_CENTERED;
    }

    double pitch = calculatePitch();
    double roll = calculateRoll();
    double pitch_abs = abs(pitch);
    double roll_abs = abs(roll);

    double range = M_PI / 4 - m_diagonal_range / 2;
    bool inPitch = pitch_abs < m_dead_zone;
    bool inRoll = roll_abs < range;

    if (std::isnan(roll))
    {
        return SENSOR_CENTERED;
    }

    if (!inPitch)
    {
        if (!inRoll)
        {
            if (roll > 0)
            {
                return pitch > 0 ? SENSOR_LEFT_UP : SENSOR_LEFT_DOWN;
            } else
            {
                return pitch > 0 ? SENSOR_RIGHT_UP : SENSOR_RIGHT_DOWN;
            }
        } else
        {
            return pitch > 0 ? SENSOR_UP : SENSOR_DOWN;
        }
    } else if (!inRoll)
    {
        return roll > 0 ? SENSOR_LEFT : SENSOR_RIGHT;
    }

    return SENSOR_CENTERED;
}

/**
 * @brief Applies calibration to queued input values
 *
 * This rotates the sensor coordinate system with the precalculated neutral
 * position rotation matrix.
 */
void JoyAccelerometerSensor::applyCalibration()
{
    double x = m_pending_value[0];
    double y = m_pending_value[1];
    double z = m_pending_value[2];

    m_pending_value[0] = m_calibration_matrix[0][0] * x + m_calibration_matrix[0][1] * y + m_calibration_matrix[0][2] * z;
    m_pending_value[1] = m_calibration_matrix[1][0] * x + m_calibration_matrix[1][1] * y + m_calibration_matrix[1][2] * z;
    m_pending_value[2] = m_calibration_matrix[2][0] * x + m_calibration_matrix[2][1] * y + m_calibration_matrix[2][2] * z;
}
