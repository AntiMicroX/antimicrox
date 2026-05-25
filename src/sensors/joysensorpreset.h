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

#include "uihelpers/joysensoriothreadhelper.h"

/**
 * @brief Defines presets for a sensor
 */
class JoySensorPreset : public QObject
{
    Q_OBJECT

  public:
    enum Preset
    {
        PRESET_NONE,
        PRESET_MOUSE,
        PRESET_MOUSE_INV_H,
        PRESET_MOUSE_INV_V,
        PRESET_MOUSE_INV_HV,
        PRESET_ARROWS,
        PRESET_WASD,
        PRESET_NUMPAD
    };

    explicit JoySensorPreset(JoySensor *sensor, QObject *parent = nullptr);

    QList<Preset> getAvailablePresets();
    Preset currentPreset();
    QString getPresetName(Preset);
    void setSensorPreset(Preset);
    JoySensorIoThreadHelper &getHelper();

  private:
    JoySensor *m_sensor;
    JoySensorIoThreadHelper m_helper;
};
