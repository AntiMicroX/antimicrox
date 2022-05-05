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

#include "joysensordirection.h"

#include <QGridLayout>

class JoySensor;
class QWidget;
class JoySensorButtonPushButton;
class JoySensorPushButton;

/**
 * @brief The sensor button mapping widget in the main window.
 *  The layout is based on a isometric 3D view with the regular
 *  XY axes and a diagonal Z axis.
 */
class SensorPushButtonGroup : public QGridLayout
{
    Q_OBJECT

  public:
    explicit SensorPushButtonGroup(JoySensor *sensor, bool keypadUnlocked, bool displayNames = false,
                                   QWidget *parent = nullptr);
    JoySensor *getSensor() const;

    bool ifDisplayNames() const;

  signals:
    void buttonSlotChanged();

  public slots:
    void toggleNameDisplay();

  private slots:
    void propagateSlotsChanged();
    void openSensorButtonDialog(JoySensorButtonPushButton *pushbutton);
    void showSensorDialog();

  private:
    JoySensor *m_sensor;
    bool m_display_names;
    bool m_keypad_unlocked;

    JoySensorButtonPushButton *m_up_button;
    JoySensorButtonPushButton *m_down_button;
    JoySensorButtonPushButton *m_left_button;
    JoySensorButtonPushButton *m_right_button;
    JoySensorButtonPushButton *m_fwd_button;
    JoySensorButtonPushButton *m_bwd_button;

    JoySensorPushButton *m_sensor_widget;

    JoySensorButtonPushButton *generateBtnToGrid(JoySensorDirection sensorDir, int gridRow, int gridCol);
};
