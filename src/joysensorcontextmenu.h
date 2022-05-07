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

#include "joysensorpreset.h"

#include <QMenu>

class JoySensor;
class QWidget;

/**
 * @brief The control stick context menu widget used by SensorPushButtonGroup
 */
class JoySensorContextMenu : public QMenu
{
    Q_OBJECT

  public:
    explicit JoySensorContextMenu(JoySensor *sensor, QWidget *parent = nullptr);
    void buildMenu();

  private slots:
    void openMouseSettingsDialog();

  private:
    JoySensor *m_sensor;
    JoySensorPreset m_preset;
};
