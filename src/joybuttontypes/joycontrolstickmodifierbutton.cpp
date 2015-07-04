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

//#include <QDebug>
#include <QString>
#include <cmath>

#include "joycontrolstick.h"
#include "joycontrolstickmodifierbutton.h"

const QString JoyControlStickModifierButton::xmlName = "stickmodifierbutton";

JoyControlStickModifierButton::JoyControlStickModifierButton(JoyControlStick *stick, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(0, originset, parentSet, parent)
{
    this->stick = stick;
}

QString JoyControlStickModifierButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = stick->getPartialName(forceFullFormat, displayNames);

    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Modifier")).append(" ");
        }

        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Modifier")).append(" ");
        }

        temp.append(defaultButtonName);
    }
    else
    {
        temp.append(tr("Modifier"));
    }
    return temp;
}

QString JoyControlStickModifierButton::getXmlName()
{
    return this->xmlName;
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickModifierButton::getDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickModifierButton::getMouseDistanceFromDeadZone()
{
    return getDistanceFromDeadZone();
}

void JoyControlStickModifierButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    Q_UNUSED(condition);
    Q_UNUSED(passive);
}

/*int JoyControlStickModifierButton::getRealJoyNumber()
{
    return index;
}
*/

JoyControlStick* JoyControlStickModifierButton::getStick()
{
    return stick;
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyControlStickModifierButton::setTurboMode(TurboMode mode)
{
    if (isPartRealAxis())
    {
        currentTurboMode = mode;
    }
}

/**
 * @brief Check if button should be considered a part of a real controller
 *     axis. Needed for some dialogs so the program won't have to resort to
 *     type checking.
 * @return Status of being part of a real controller axis
 */
bool JoyControlStickModifierButton::isPartRealAxis()
{
    return true;
}

bool JoyControlStickModifierButton::isModifierButton()
{
    return true;
}

double JoyControlStickModifierButton::getAccelerationDistance()
{
    double temp = stick->getAbsoluteRawDistance();
    return temp;
}

double JoyControlStickModifierButton::getLastAccelerationDistance()
{
    double temp = stick->calculateLastAccelerationDirectionalDistance();
    return temp;
}

double JoyControlStickModifierButton::getLastMouseDistanceFromDeadZone()
{
    return stick->calculateLastDirectionalDistance();
}
