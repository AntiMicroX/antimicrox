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

#include "joybuttontypes/joysensorbutton.h"

/**
 * @brief Represents an accelerometer mapping in a SetJoystick
 */
class JoyAccelerometerButton : public JoySensorButton
{
    Q_OBJECT

  public:
    explicit JoyAccelerometerButton(JoySensor *sensor, int sdl_button_index, int originset, SetJoystick *parentSet,
                                    QObject *parent);

    virtual QString getDirectionName() const override;
};
