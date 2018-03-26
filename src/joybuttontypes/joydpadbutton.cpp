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

#include "joydpadbutton.h"
#include "joydpad.h"
#include "setjoystick.h"
#include "event.h"

#include <QDebug>

const QString JoyDPadButton::xmlName = "dpadbutton";

// Initially, qualify direction as the button's index
JoyDPadButton::JoyDPadButton(int direction, int originset, JoyDPad* dpad, SetJoystick *parentSet, QObject *parent) :
    JoyButton(direction, originset, parentSet, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->direction = direction;
    this->dpad = dpad;
}

QString JoyDPadButton::getDirectionName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString label = QString ();
    if (direction == DpadUp)
    {
        label.append(trUtf8("Up"));
    }
    else if (direction == DpadDown)
    {
        label.append(trUtf8("Down"));
    }
    else if (direction == DpadLeft)
    {
        label.append(trUtf8("Left"));
    }
    else if (direction == DpadRight)
    {
        label.append(trUtf8("Right"));
    }
    else if (direction == DpadLeftUp)
    {
        label.append(trUtf8("Up")).append("+").append(trUtf8("Left"));
    }
    else if (direction == DpadLeftDown)
    {
        label.append(trUtf8("Down")).append("+").append(trUtf8("Left"));
    }
    else if (direction == DpadRightUp)
    {
        label.append(trUtf8("Up")).append("+").append(trUtf8("Right"));
    }
    else if (direction == DpadRightDown)
    {
        label.append(trUtf8("Down")).append("+").append(trUtf8("Right"));
    }

    return label;
}

QString JoyDPadButton::getXmlName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->xmlName;
}

int JoyDPadButton::getRealJoyNumber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return index;
}

QString JoyDPadButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = dpad->getName().append(" - ");
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
        temp.append(trUtf8("Button")).append(" ");
        temp.append(getDirectionName());
    }
    return temp;
}


void JoyDPadButton::reset()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::reset();
}

void JoyDPadButton::reset(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_UNUSED(index);
    reset();
}

void JoyDPadButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(index, this->dpad->getJoyNumber(), setSelection, condition);
        }
        else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->dpad->getJoyNumber(), setSelection, SetChangeDisabled);
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
        buildActiveZoneSummaryString();
        emit propertyUpdated();
    }
}

JoyDPad* JoyDPadButton::getDPad()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return dpad;
}

int JoyDPadButton::getDirection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return direction;
}
