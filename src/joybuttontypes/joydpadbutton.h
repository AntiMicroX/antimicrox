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

#ifndef JOYDPADBUTTON_H
#define JOYDPADBUTTON_H

#include "joybutton.h"

class JoyDPad;

class JoyDPadButton : public JoyButton
{
    Q_OBJECT
public:
    JoyDPadButton(int direction, int originset, JoyDPad* dpad, SetJoystick *parentSet, QObject *parent=0);

    QString getDirectionName();
    int getDirection();
    virtual int getRealJoyNumber();
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    JoyDPad *getDPad();

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);

    enum JoyDPadDirections {
        DpadCentered = 0, DpadUp = 1, DpadRight = 2,
        DpadDown = 4, DpadLeft = 8, DpadRightUp = 3,
        DpadRightDown = 6, DpadLeftUp = 9, DpadLeftDown = 12
    };
    static const QString xmlName;

protected:
    int direction;
    JoyDPad *dpad;

signals:
    void setAssignmentChanged(int current_button, int dpad_index, int associated_set, int mode);

public slots:
    virtual void reset();
    virtual void reset(int index);
};

#endif // JOYDPADBUTTON_H
