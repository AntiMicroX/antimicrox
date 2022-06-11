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

#include "inputdevicecalibration.h"
#include "inputdevice.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * @brief Returns true if the two CalibrationData structs references the same physical,
 *   e.g. stick or sensor input on any controller. Otherwise, it returns false.
 */
bool CalibrationData::referencesSameInput(const CalibrationData &rhs) const
{
    if (type != rhs.type)
        return false;

    if (type == CALIBRATION_DATA_STICK)
        return stick.index == rhs.stick.index;
    else if (type == CALIBRATION_DATA_ACCELEROMETER)
        return true;
    else if (type == CALIBRATION_DATA_GYROSCOPE)
        return true;
    else
        return false;
}

InputDeviceCalibration::InputDeviceCalibration(InputDevice *device)
    : m_device(device)
{
}

/**
 * @brief Updates the stored calibration for the given stick from the parent controller.
 * @param[in] index Stick index
 * @param[in] offsetX Offset value for X axis
 * @param[in] gainX Gain value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] gainY Gain value for Y axis
 */
void InputDeviceCalibration::setStickCalibration(int index, double offsetX, double gainX, double offsetY, double gainY)
{
    CalibrationData calibration;
    calibration.type = CALIBRATION_DATA_STICK;

    StickCalibrationData &stick = calibration.stick;
    stick.index = index;
    stick.offsetX = offsetX;
    stick.gainX = gainX;
    stick.offsetY = offsetY;
    stick.gainY = gainY;

    setCalibration(m_device->getUniqueIDString(), calibration);
}

/**
 * @brief Updates the stored calibration for the given accelerometer from the parent controller.
 * @param[in] orientationX X coordinate of the neutral orientation vector
 * @param[in] orientationY Y coordinate of the neutral orientation vector
 * @param[in] orientationZ Z coordinate of the neutral orientation vector
 */
void InputDeviceCalibration::setAccelerometerCalibration(double orientationX, double orientationY, double orientationZ)
{
    CalibrationData calibration;
    calibration.type = CALIBRATION_DATA_ACCELEROMETER;

    AccelerometerCalibrationData &accelerometer = calibration.accelerometer;
    accelerometer.orientationX = orientationX;
    accelerometer.orientationY = orientationY;
    accelerometer.orientationZ = orientationZ;
    setCalibration(m_device->getUniqueIDString(), calibration);
}

/**
 * @brief Updates the stored calibration for the given gyroscope from the parent controller.
 * @param[in] offsetX Offset value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] offsetZ Offset value for Z axis
 */
void InputDeviceCalibration::setGyroscopeCalibration(double offsetX, double offsetY, double offsetZ)
{
    CalibrationData calibration;
    calibration.type = CALIBRATION_DATA_GYROSCOPE;

    GyroscopeCalibrationData &gyroscope = calibration.gyroscope;
    gyroscope.offsetX = offsetX;
    gyroscope.offsetY = offsetY;
    gyroscope.offsetZ = offsetZ;
    setCalibration(m_device->getUniqueIDString(), calibration);
}

/**
 * @brief Updated the given CalibrationData structure of the controller with
 *   the given ID in the calibration storage backend.
 * @param[in] id ID of the device to which the calibration data belongs to
 * @param[in] new_calibration The CalibrationData structure to be stored
 */
void InputDeviceCalibration::setCalibration(QString id, CalibrationData new_calibration)
{
    for (auto &calibration : m_data[id])
    {
        if (calibration.referencesSameInput(new_calibration))
        {
            calibration = new_calibration;
            return;
        }
    }

    m_data[id].append(new_calibration);
}
