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

#include <QHash>

class InputDevice;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Defines the possible CalibrationData union types
 */
enum CalibrationDataType
{
    CALIBRATION_DATA_STICK,
    CALIBRATION_DATA_ACCELEROMETER,
    CALIBRATION_DATA_GYROSCOPE
};

/**
 * @brief The calibration data for a stick
 */
struct StickCalibrationData
{
    int index;
    double offsetX;
    double gainX;
    double offsetY;
    double gainY;
};

/**
 * @brief The calibration data for an accelerometer
 */
struct AccelerometerCalibrationData
{
    double orientationX;
    double orientationY;
    double orientationZ;
};

/**
 * @brief The calibration data for a gyroscope
 */
struct GyroscopeCalibrationData
{
    double offsetX;
    double offsetY;
    double offsetZ;
};

/**
 * @brief Stores the calibration data for one physical device, e.g. a stick or gyroscope.
 */
struct CalibrationData
{
    CalibrationDataType type;
    union
    {
        StickCalibrationData stick;
        AccelerometerCalibrationData accelerometer;
        GyroscopeCalibrationData gyroscope;
    };

    bool referencesSameInput(const CalibrationData &rhs) const;
};

/**
 * @brief Calibration storage backend which can store multiple calibration items for different controllers.
 */
class InputDeviceCalibration
{
  public:
    explicit InputDeviceCalibration(InputDevice *device);

    void setStickCalibration(int index, double offsetX, double gainX, double offsetY, double gainY);
    void setAccelerometerCalibration(double orientationX, double orientationY, double orientationZ);
    void setGyroscopeCalibration(double offsetX, double offsetY, double offsetZ);
    void applyCalibrations() const;

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml) const;

  private:
    void setCalibration(QString id, CalibrationData new_calibration);
    QHash<QString, QList<CalibrationData>> m_data;

    InputDevice *m_device;
};
