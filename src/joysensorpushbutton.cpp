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

#include "joysensorpushbutton.h"

#include "joysensor.h"
#include "joysensorcontextmenu.h"

#include <QDebug>

JoySensorPushButton::JoySensorPushButton(JoySensor *sensor, bool displayNames, QWidget *parent)
    : FlashButtonWidget(displayNames, parent)
    , m_sensor(sensor)
{
    refreshLabel();

    tryFlash();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoySensorPushButton::customContextMenuRequested, this, &JoySensorPushButton::showContextMenu);

    connect(m_sensor, &JoySensor::active, this, &JoySensorPushButton::flash, Qt::QueuedConnection);
    connect(m_sensor, &JoySensor::released, this, &JoySensorPushButton::unflash, Qt::QueuedConnection);
    connect(m_sensor, &JoySensor::sensorNameChanged, this, &JoySensorPushButton::refreshLabel);
}

/**
 * @brief Get the underlying JoySensor object.
 */
JoySensor *JoySensorPushButton::getSensor() const { return m_sensor; }

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoySensorPushButton::generateLabel()
{
    QString temp = QString();
    if (!m_sensor->getSensorName().isEmpty() && ifDisplayNames())
        temp.append(m_sensor->getPartialName(false, true));
    else
        temp.append(m_sensor->getPartialName(false));

    qDebug() << "Name of joy sensor push button: " << temp;

    return temp;
}

/**
 * @brief Disables highlight when the sensor axis is moved
 */
void JoySensorPushButton::disableFlashes()
{
    disconnect(m_sensor, &JoySensor::active, this, &JoySensorPushButton::flash);
    disconnect(m_sensor, &JoySensor::released, this, &JoySensorPushButton::unflash);
    unflash();
}

/**
 * @brief Enables highlight when the sensor axis is moved
 */
void JoySensorPushButton::enableFlashes()
{
    connect(m_sensor, &JoySensor::active, this, &JoySensorPushButton::flash, Qt::QueuedConnection);
    connect(m_sensor, &JoySensor::released, this, &JoySensorPushButton::unflash, Qt::QueuedConnection);
}

/**
 * @brief Shows sensor context menu
 */
void JoySensorPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = mapToGlobal(point);
    JoySensorContextMenu *contextMenu = new JoySensorContextMenu(m_sensor, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

/**
 * @brief Highlights the button when sensor is not centered
 */
void JoySensorPushButton::tryFlash()
{
    if (m_sensor->getCurrentDirection() != JoySensorDirection::SENSOR_CENTERED)
        flash();
}
