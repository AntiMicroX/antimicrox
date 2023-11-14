/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#ifndef SPRINGMOUSEMOVEINFO_H
#define SPRINGMOUSEMOVEINFO_H

namespace PadderCommon {
typedef struct _springModeInfo
{
    // Displacement of the X axis
    double displacementX;
    // Displacement of the Y axis
    double displacementY;

    // Width and height of the spring mode box
    int width;  // unsigned
    int height; // unsigned
    // Should the cursor not move around the center
    // of the screen.
    bool relative;
    int screen; //-1 for default screen
    double springDeadX;
    double springDeadY;

} springModeInfo;
} // namespace PadderCommon

#endif // SPRINGMOUSEMOVEINFO_H
