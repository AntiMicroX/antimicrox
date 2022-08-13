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

/**
 * @brief Enum of the supported haptic trigger effect modes of the
 *   PS5 controller.
 *   See https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db
 */
enum HapticTriggerModePs5
{
    HAPTIC_TRIGGER_NONE = 0x05,
    HAPTIC_TRIGGER_CLICK = 0x25,
    HAPTIC_TRIGGER_RIGID = 0x21,
    HAPTIC_TRIGGER_VIBRATION = 0x26
};
