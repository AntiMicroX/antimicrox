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
 * @brief Applies all applicable stored calibration values to the individual
 *   input elements of the parent controller
 */
void InputDeviceCalibration::applyCalibrations() const
{
    QString id = m_device->getUniqueIDString();
    for (const auto &calibration : m_data[id])
    {
        if (calibration.type == CALIBRATION_DATA_STICK)
        {
            const StickCalibrationData &data = calibration.stick;
            m_device->applyStickCalibration(data.index, data.offsetX, data.gainX, data.offsetY, data.gainY);
        } else if (calibration.type == CALIBRATION_DATA_ACCELEROMETER)
        {
            const AccelerometerCalibrationData &data = calibration.accelerometer;
            m_device->applyAccelerometerCalibration(data.orientationX, data.orientationY, data.orientationZ);
        } else if (calibration.type == CALIBRATION_DATA_GYROSCOPE)
        {
            const GyroscopeCalibrationData &data = calibration.gyroscope;
            m_device->applyGyroscopeCalibration(data.offsetX, data.offsetY, data.offsetZ);
        }
    }
}

/**
 * @brief Reads all calibration values from the given XML stream into the internal calibration data storage
 * @param QXmlStreamReader instance that will be used to read calibration values.
 */
void InputDeviceCalibration::readConfig(QXmlStreamReader *xml)
{
    while (xml->isStartElement() && (xml->name().toString() == "calibration"))
    {
        QString id = xml->attributes().value("device").toString();
        if (id.isEmpty())
            id = m_device->getUniqueIDString();
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "calibration")))
        {
            CalibrationData calibration;
            if ((xml->name().toString() == "stick"))
            {
                calibration.type = CALIBRATION_DATA_STICK;
                calibration.stick.index = xml->attributes().value("index").toString().toInt();
                calibration.stick.offsetX = xml->attributes().value("offsetx").toString().toDouble();
                calibration.stick.gainX = xml->attributes().value("gainx").toString().toDouble();
                calibration.stick.offsetY = xml->attributes().value("offsety").toString().toDouble();
                calibration.stick.gainY = xml->attributes().value("gainy").toString().toDouble();
                setCalibration(id, calibration);
            } else if ((xml->name().toString() == "accelerometer"))
            {
                calibration.type = CALIBRATION_DATA_ACCELEROMETER;
                calibration.accelerometer.orientationX = xml->attributes().value("orientationx").toString().toDouble();
                calibration.accelerometer.orientationY = xml->attributes().value("orientationy").toString().toDouble();
                calibration.accelerometer.orientationZ = xml->attributes().value("orientationz").toString().toDouble();
                setCalibration(id, calibration);
            } else if ((xml->name().toString() == "gyroscope"))
            {
                calibration.type = CALIBRATION_DATA_GYROSCOPE;
                calibration.gyroscope.offsetX = xml->attributes().value("offsetx").toString().toDouble();
                calibration.gyroscope.offsetY = xml->attributes().value("offsety").toString().toDouble();
                calibration.gyroscope.offsetZ = xml->attributes().value("offsetz").toString().toDouble();
                setCalibration(id, calibration);
            }
            xml->skipCurrentElement();
            xml->readNextStartElement();
        }
    }
}

/**
 * @brief Writes all stored calibration values from the internal storage into the given XML stream.
 * @param QXmlStreamWriter instance that will be used to write calibration values.
 */
void InputDeviceCalibration::writeConfig(QXmlStreamWriter *xml) const
{
    for (auto device = m_data.cbegin(); device != m_data.cend(); ++device)
    {
        xml->writeStartElement("calibration");
        xml->writeAttribute("device", device.key());
        for (const auto calibration : *device)
        {

            if (calibration.type == CALIBRATION_DATA_STICK)
            {
                const StickCalibrationData &data = calibration.stick;
                xml->writeStartElement("stick");
                xml->writeAttribute("index", QString::number(data.index));
                xml->writeAttribute("offsetx", QString::number(data.offsetX));
                xml->writeAttribute("gainx", QString::number(data.gainX));
                xml->writeAttribute("offsety", QString::number(data.offsetY));
                xml->writeAttribute("gainy", QString::number(data.gainY));
                xml->writeEndElement();
            } else if (calibration.type == CALIBRATION_DATA_ACCELEROMETER)
            {
                const AccelerometerCalibrationData &data = calibration.accelerometer;
                xml->writeStartElement("accelerometer");
                xml->writeAttribute("orientationx", QString::number(data.orientationX));
                xml->writeAttribute("orientationy", QString::number(data.orientationY));
                xml->writeAttribute("orientationz", QString::number(data.orientationZ));
                xml->writeEndElement();
            } else if (calibration.type == CALIBRATION_DATA_GYROSCOPE)
            {
                const GyroscopeCalibrationData &data = calibration.gyroscope;
                xml->writeStartElement("gyroscope");
                xml->writeAttribute("offsetx", QString::number(data.offsetX));
                xml->writeAttribute("offsety", QString::number(data.offsetY));
                xml->writeAttribute("offsetz", QString::number(data.offsetZ));
                xml->writeEndElement();
            }
        }
        xml->writeEndElement();
    }
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
