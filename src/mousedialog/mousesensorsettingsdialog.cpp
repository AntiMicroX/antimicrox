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

#include "mousesensorsettingsdialog.h"

#include "common.h"
#include "inputdevice.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joysensor.h"
#include "setjoystick.h"

#include <QComboBox>
#include <QDebug>
#include <QHashIterator>
#include <QSpinBox>

MouseSensorSettingsDialog::MouseSensorSettingsDialog(JoySensor *sensor, QWidget *parent)
    : MouseSettingsDialog(parent)
    , m_sensor(sensor)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->topGroupBox->setVisible(false);
    ui->springGroupBox->setVisible(false);
    ui->extraAccelerationGroupBox->setVisible(false);
    ui->sensLabel->setVisible(false);
    ui->sensitivityDoubleSpinBox->setVisible(false);
    ui->easingDurationLabel->setVisible(false);
    ui->easingDoubleSpinBox->setVisible(false);

    calculateMouseSpeedPreset();

    updateWindowTitleSensorName();
    calculateWheelSpeedPreset();

    changeSettingsWidgetStatus(ui->accelerationComboBox->currentIndex());

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSensorSettingsDialog::updateConfigHorizontalSpeed);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSensorSettingsDialog::updateConfigVerticalSpeed);

    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSensorSettingsDialog::updateWheelSpeedHorizontalSpeed);
    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSensorSettingsDialog::updateWheelSpeedVerticalSpeed);
}

/**
 * @brief Gets the underlying JoySensor object
 */
JoySensor *MouseSensorSettingsDialog::getSensor() const { return m_sensor; }

/**
 * @brief Get the maximum mouse speed used by the buttons of the associated
 *  sensor and set the UI values to it.
 */
void MouseSensorSettingsDialog::calculateMouseSpeedPreset()
{
    auto buttons = m_sensor->getButtons();
    int mouseSpeedX = 0;
    int mouseSpeedY = 0;
    for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
    {
        JoySensorButton *button = iter.value();
        mouseSpeedX = qMax(mouseSpeedX, button->getMouseSpeedX());
        mouseSpeedY = qMax(mouseSpeedY, button->getMouseSpeedY());
    }

    ui->horizontalSpinBox->setValue(mouseSpeedX);
    ui->verticalSpinBox->setValue(mouseSpeedY);
}

/**
 * @brief Get the maximum mouse wheel speed used by the buttons of the associated
 *  sensor and set the UI values to it.
 */
void MouseSensorSettingsDialog::calculateWheelSpeedPreset()
{
    auto buttons = m_sensor->getButtons();
    int wheelSpeedX = 0;
    int wheelSpeedY = 0;
    for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
    {
        JoySensorButton *button = iter.value();
        wheelSpeedX = qMax(wheelSpeedX, button->getWheelSpeedX());
        wheelSpeedY = qMax(wheelSpeedY, button->getWheelSpeedY());
    }

    ui->wheelHoriSpeedSpinBox->setValue(wheelSpeedX);
    ui->wheelVertSpeedSpinBox->setValue(wheelSpeedY);
}

/**
 * @brief Shows the sensor name in dialog title
 */
void MouseSensorSettingsDialog::updateWindowTitleSensorName()
{
    QString temp = QString(tr("Mouse Settings")).append(" - ");

    if (!m_sensor->getSensorName().isEmpty())
        temp.append(m_sensor->getPartialName(false, true));
    else
        temp.append(m_sensor->getPartialName());

    if (m_sensor->getParentSet()->getIndex() != 0)
    {
        int setIndex = m_sensor->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = m_sensor->getParentSet()->getName();
        if (!setName.isEmpty())
            temp.append(": ").append(setName);

        temp.append("]");
    }

    setWindowTitle(temp);
}

/**
 * @brief Horizontal mouse speed change UI event handler
 *  Updates horizontal mouse speed on all buttons of the associated sensor.
 */
void MouseSensorSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    auto buttons = m_sensor->getButtons();
    for (auto iter = buttons->begin(); iter != buttons->end(); ++iter)
    {
        JoySensorButton *button = iter.value();
        button->setMouseSpeedX(value);
    }
}

/**
 * @brief Vertical mouse speed change UI event handler
 *  Updates vertical mouse speed on all buttons of the associated sensor.
 */
void MouseSensorSettingsDialog::updateConfigVerticalSpeed(int value)
{
    auto buttons = m_sensor->getButtons();
    for (auto iter = buttons->begin(); iter != buttons->end(); ++iter)
    {
        JoySensorButton *button = iter.value();
        button->setMouseSpeedY(value);
    }
}

/**
 * @brief Horizontal mouse wheel speed change UI event handler
 *  Updates horizontal mouse wheel speed on all buttons of the associated sensor.
 */
void MouseSensorSettingsDialog::updateWheelSpeedHorizontalSpeed(int value)
{
    auto buttons = m_sensor->getButtons();
    for (auto iter = buttons->begin(); iter != buttons->end(); ++iter)
    {
        JoySensorButton *button = iter.value();
        button->setWheelSpeed(value, 'X');
    }
}

/**
 * @brief Vertical mouse wheel speed change UI event handler
 *  Updates vertical mouse wheel speed on all buttons of the associated sensor.
 */
void MouseSensorSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    auto buttons = m_sensor->getButtons();
    for (auto iter = buttons->begin(); iter != buttons->end(); ++iter)
    {
        JoySensorButton *button = iter.value();
        button->setWheelSpeed(value, 'Y');
    }
}

/**
 * @brief Not used for sensors but necessary to implement because it is an
 *  abstract function in the parent class.
 */
void MouseSensorSettingsDialog::changeMouseMode(int index) { Q_UNUSED(index); }

/**
 * @brief Not used for sensors but necessary to implement because it is an
 *  abstract function in the parent class.
 */
void MouseSensorSettingsDialog::changeMouseCurve(int index) { Q_UNUSED(index); }
