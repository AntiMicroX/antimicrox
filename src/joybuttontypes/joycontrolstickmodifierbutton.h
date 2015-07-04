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

#ifndef JOYCONTROLSTICKMODIFIERBUTTON_H
#define JOYCONTROLSTICKMODIFIERBUTTON_H

#include "joybuttontypes/joygradientbutton.h"

class JoyControlStick;

class JoyControlStickModifierButton : public JoyGradientButton
{
    Q_OBJECT
public:
    explicit JoyControlStickModifierButton(JoyControlStick *stick, int originset, SetJoystick *parentSet, QObject *parent = 0);

    //virtual int getRealJoyNumber();
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual double getLastMouseDistanceFromDeadZone();

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyControlStick *getStick();
    virtual void setTurboMode(TurboMode mode);
    virtual bool isPartRealAxis();
    virtual bool isModifierButton();

    virtual double getAccelerationDistance();
    virtual double getLastAccelerationDistance();

    static const QString xmlName;

protected:
    JoyControlStick *stick;

};

#endif // JOYCONTROLSTICKMODIFIERBUTTON_H
