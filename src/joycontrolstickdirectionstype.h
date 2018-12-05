/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#ifndef JOYCONTROLSTICKDIRECTIONSTYPE_H
#define JOYCONTROLSTICKDIRECTIONSTYPE_H

class JoyStickDirectionsType {
public:
    enum JoyStickDirections {
        StickCentered = 0, StickUp = 1, StickRight = 3,
        StickDown = 5, StickLeft = 7, StickRightUp = 2,
        StickRightDown = 4, StickLeftUp = 8, StickLeftDown = 6
    };
};

#endif // JOYCONTROLSTICKDIRECTIONSTYPE_H
