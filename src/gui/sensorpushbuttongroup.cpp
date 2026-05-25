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

#include "sensorpushbuttongroup.h"

#include "buttoneditdialog.h"
#include "inputdevice.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joysensor.h"
#include "joysensorbuttonpushbutton.h"
#include "joysensoreditdialog.h"
#include "joysensorpushbutton.h"

#include <QDebug>
#include <QHash>
#include <QWidget>

SensorPushButtonGroup::SensorPushButtonGroup(JoySensor *sensor, bool keypadUnlocked, bool displayNames, QWidget *parent)
    : QGridLayout(parent)
    , m_sensor(sensor)
    , m_display_names(displayNames)
    , m_keypad_unlocked(keypadUnlocked)
{
    m_left_button = generateBtnToGrid(SENSOR_LEFT, 1, 0);
    m_right_button = generateBtnToGrid(SENSOR_RIGHT, 1, 2);
    m_up_button = generateBtnToGrid(SENSOR_UP, 0, 1);
    m_down_button = generateBtnToGrid(SENSOR_DOWN, 2, 1);
    m_bwd_button = generateBtnToGrid(SENSOR_BWD, 0, 2);

    if (m_sensor->getType() == GYROSCOPE)
        m_fwd_button = generateBtnToGrid(SENSOR_FWD, 2, 0);
    else
        m_fwd_button = nullptr;

    m_sensor_widget = new JoySensorPushButton(m_sensor, m_display_names, parentWidget());
    m_sensor_widget->setIcon(
        QIcon::fromTheme(QString::fromUtf8("games_config_options"), QIcon(":/images/actions/games_config_options.png")));

    connect(m_sensor_widget, &JoySensorPushButton::clicked, this, &SensorPushButtonGroup::showSensorDialog);

    addWidget(m_sensor_widget, 1, 1);
}

/**
 * @brief Generates a new push button at the given grid coordinates
 * @returns Newly created push button
 */
JoySensorButtonPushButton *SensorPushButtonGroup::generateBtnToGrid(JoySensorDirection sensorDir, int gridRow, int gridCol)
{
    JoySensorButton *button = m_sensor->getButtons()->value(sensorDir);
    JoySensorButtonPushButton *pushbutton = new JoySensorButtonPushButton(button, m_display_names, parentWidget());

    connect(pushbutton, &JoySensorButtonPushButton::clicked, this,
            [this, pushbutton] { openSensorButtonDialog(pushbutton); });

    button->establishPropertyUpdatedConnections();
    connect(button, &JoySensorButton::slotsChanged, this, &SensorPushButtonGroup::propagateSlotsChanged);

    addWidget(pushbutton, gridRow, gridCol);
    return pushbutton;
}

void SensorPushButtonGroup::propagateSlotsChanged() { emit buttonSlotChanged(); }

/**
 * @brief Get the underlying JoySensor object.
 */
JoySensor *SensorPushButtonGroup::getSensor() const { return m_sensor; }

/**
 * @brief Shows the button mapping dialog for the given direction button
 */
void SensorPushButtonGroup::openSensorButtonDialog(JoySensorButtonPushButton *pushbutton)
{
    ButtonEditDialog *dialog = new ButtonEditDialog(pushbutton->getButton(), m_sensor->getParentSet()->getInputDevice(),
                                                    m_keypad_unlocked, parentWidget());
    dialog->show();
}

/**
 * @brief Shows the sensor settings dialog
 */
void SensorPushButtonGroup::showSensorDialog()
{
    JoySensorEditDialog *dialog = new JoySensorEditDialog(m_sensor, parentWidget());
    dialog->show();
}

void SensorPushButtonGroup::toggleNameDisplay()
{
    m_display_names = !m_display_names;

    m_up_button->toggleNameDisplay();
    m_down_button->toggleNameDisplay();
    m_left_button->toggleNameDisplay();
    m_right_button->toggleNameDisplay();
    m_bwd_button->toggleNameDisplay();
    if (m_fwd_button != nullptr)
        m_fwd_button->toggleNameDisplay();

    m_sensor_widget->toggleNameDisplay();
}

bool SensorPushButtonGroup::ifDisplayNames() const { return m_display_names; }
