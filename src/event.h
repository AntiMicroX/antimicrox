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

#ifndef EVENT_H
#define EVENT_H

#include <QString>

#include "joybuttonslot.h"
//#include "mousehelper.h"
#include "springmousemoveinfo.h"
#include "common.h"

void sendevent (JoyButtonSlot *slot, bool pressed=true);
void sendevent(int code1, int code2);
void sendSpringEventRefactor(PadderCommon::springModeInfo *fullSpring,
                        PadderCommon::springModeInfo *relativeSpring=0,
                        int* const mousePosX=0, int* const mousePos=0);

void sendSpringEvent(PadderCommon::springModeInfo *fullSpring,
                     PadderCommon::springModeInfo *relativeSpring=0,
                     int* const mousePosX=0, int* const mousePos=0);

int X11KeySymToKeycode(QString key);
QString keycodeToKeyString(int keycode, unsigned int alias=0);
unsigned int X11KeyCodeToX11KeySym(unsigned int keycode);
QString keysymToKeyString(int keysym, unsigned int alias=0);

#endif // EVENT_H
