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

#ifndef JOYCONTROLSTICKBUTTON_H
#define JOYCONTROLSTICKBUTTON_H\

#include <QString>

#include "joybuttontypes/joygradientbutton.h"
#include "joycontrolstickdirectionstype.h"

class JoyControlStick;

class JoyControlStickButton : public JoyGradientButton
{
    Q_OBJECT
public:
    explicit JoyControlStickButton(JoyControlStick *stick, int index, int originset, SetJoystick *parentSet, QObject *parent = 0);
    explicit JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index, int originset, SetJoystick *parentSet, QObject *parent = 0);

    virtual int getRealJoyNumber();
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    QString getDirectionName();
    JoyStickDirectionsType::JoyStickDirections getDirection();
    virtual double getDistanceFromDeadZone();

    virtual double getMouseDistanceFromDeadZone();
    virtual double getLastMouseDistanceFromDeadZone();

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyControlStick *getStick();
    virtual void setTurboMode(TurboMode mode);
    virtual bool isPartRealAxis();
    virtual QString getActiveZoneSummary();
    virtual QString getCalculatedActiveZoneSummary();

    virtual double getAccelerationDistance();
    virtual double getLastAccelerationDistance();

    static const QString xmlName;

protected:
    virtual double getCurrentSpringDeadCircle();

    JoyControlStick *stick;

signals:
    void setAssignmentChanged(int current_button, int axis_index, int associated_set, int mode);

};

#endif // JOYCONTROLSTICKBUTTON_H
