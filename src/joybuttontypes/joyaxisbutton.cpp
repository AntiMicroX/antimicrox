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

#include "joyaxisbutton.h"
#include "joyaxis.h"
#include "joybutton.h"
#include "setjoystick.h"
#include "vdpad.h"
#include "event.h"

#include <cmath>

#include <QDebug>

const QString JoyAxisButton::xmlName = "axisbutton";

JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(index, originset, parentSet, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    this->axis = axis;
}

QString JoyAxisButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString(axis->getPartialName(forceFullFormat, displayNames));
    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(trUtf8("Button")).append(" ");
        }
        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(trUtf8("Button")).append(" ");
        }
        temp.append(defaultButtonName);
    }
    else
    {
        QString buttontype = QString();
        if (index == 0)
        {
            buttontype = trUtf8("Negative");
        }
        else if (index == 1)
        {
            buttontype = trUtf8("Positive");
        }
        else
        {
            buttontype = trUtf8("Unknown");
        }

        temp.append(trUtf8("Button")).append(" ").append(buttontype);
    }

    return temp;
}

QString JoyAxisButton::getXmlName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->xmlName;
}

void JoyAxisButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, condition);
        }
        else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, SetChangeDisabled);
        }

        setSelectionCondition = condition;
    }
    else if (passive)
    {
        setSelectionCondition = condition;
    }

    if (setSelectionCondition == SetChangeDisabled)
    {
        setChangeSetSelection(-1);
    }

    if (setSelectionCondition != oldCondition)
    {
        if (updateActiveString)
        {
            buildActiveZoneSummaryString();
        }

        emit propertyUpdated();
    }
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyAxisButton::getDistanceFromDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return axis->getDistanceFromDeadZone();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyAxisButton::getMouseDistanceFromDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->getDistanceFromDeadZone();
}

void JoyAxisButton::setVDPad(VDPad *vdpad)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (axis->isPartControlStick())
    {
        axis->removeControlStick();
    }

    JoyButton::setVDPad(vdpad);
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyAxisButton::setTurboMode(TurboMode mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
bool JoyAxisButton::isPartRealAxis()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return true;
}

double JoyAxisButton::getAccelerationDistance()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double distance = 0.0;
    distance = axis->getRawDistance(axis->getCurrentThrottledValue());
    return distance;
}

double JoyAxisButton::getLastAccelerationDistance()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double distance = 0.0;
    distance = axis->getRawDistance(axis->getLastKnownThrottleValue());
    /*if (axis->getAxisButtonByValue(axis->getLastKnownThrottleValue()) == this)
    {
        distance = axis->getRawDistance(axis->getLastKnownThrottleValue());
    }
    */

    return distance;
}

double JoyAxisButton::getLastMouseDistanceFromDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double distance = 0.0;
    if (axis->getAxisButtonByValue(axis->getLastKnownThrottleValue()) == this)
    {
        distance = axis->getDistanceFromDeadZone(axis->getLastKnownThrottleValue());
    }

    return distance;
}


JoyAxis* JoyAxisButton::getAxis() const {

    return axis;
}
