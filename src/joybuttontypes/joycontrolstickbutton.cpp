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
#include <cmath>
#include <QStringList>

#include "joycontrolstickbutton.h"
#include "joycontrolstick.h"
#include "event.h"

const QString JoyControlStickButton::xmlName = "stickbutton";

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(index, originset, parentSet, parent)
{
    this->stick = stick;
}

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton((int)index, originset, parentSet, parent)
{
    this->stick = stick;
}

QString JoyControlStickButton::getDirectionName()
{
    QString label = QString();
    if (index == JoyControlStick::StickUp)
    {
        label.append(tr("Up"));
    }
    else if (index == JoyControlStick::StickDown)
    {
        label.append(tr("Down"));
    }
    else if (index == JoyControlStick::StickLeft)
    {
        label.append(tr("Left"));
    }
    else if (index == JoyControlStick::StickRight)
    {
        label.append(tr("Right"));
    }
    else if (index == JoyControlStick::StickLeftUp)
    {
        label.append(tr("Up")).append("+").append(tr("Left"));
    }
    else if (index == JoyControlStick::StickLeftDown)
    {
        label.append(tr("Down")).append("+").append(tr("Left"));
    }
    else if (index == JoyControlStick::StickRightUp)
    {
        label.append(tr("Up")).append("+").append(tr("Right"));
    }
    else if (index == JoyControlStick::StickRightDown)
    {
        label.append(tr("Down")).append("+").append(tr("Right"));
    }

    return label;
}

QString JoyControlStickButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = stick->getPartialName(forceFullFormat, displayNames);

    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(defaultButtonName);
    }
    else
    {
        temp.append(tr("Button")).append(" ");
        temp.append(getDirectionName());
    }
    return temp;
}

QString JoyControlStickButton::getXmlName()
{
    return this->xmlName;
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickButton::getDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickButton::getMouseDistanceFromDeadZone()
{
    return stick->calculateMouseDirectionalDistance(this);
}

void JoyControlStickButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    SetChangeCondition oldCondition = setSelectionCondition;

    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, this->stick->getIndex(), setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->stick->getIndex(), setSelection, SetChangeDisabled);
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

int JoyControlStickButton::getRealJoyNumber()
{
    return index;
}

JoyControlStick* JoyControlStickButton::getStick()
{
    return stick;
}

JoyStickDirectionsType::JoyStickDirections JoyControlStickButton::getDirection()
{
    return static_cast<JoyStickDirectionsType::JoyStickDirections>(index);
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyControlStickButton::setTurboMode(TurboMode mode)
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
bool JoyControlStickButton::isPartRealAxis()
{
    return true;
}

double JoyControlStickButton::getLastAccelerationDistance()
{
    double temp = stick->calculateLastAccelerationButtonDistance(this);
    return temp;
}

double JoyControlStickButton::getAccelerationDistance()
{
    double temp = stick->calculateAccelerationDistance(this);
    return temp;
}

/**
 * @brief Generate a string that represents slots that will be activated or
 *     slots that are currently active if a button is pressed
 * @return String of currently applicable slots for a button
 */
QString JoyControlStickButton::getActiveZoneSummary()
{
    QList<JoyButtonSlot*> tempList;
    JoyControlStickModifierButton *tempButton = stick->getModifierButton();
    /*if (tempButton && tempButton->getButtonState() &&
          tempButton->hasActiveSlots() && getButtonState())
    {
        QList<JoyButtonSlot*> activeModifierSlots = tempButton->getActiveZoneList();
        tempList.append(activeModifierSlots);
    }
    */

    tempList.append(getActiveZoneList());
    QString temp = buildActiveZoneSummary(tempList);
    return temp;
}

QString JoyControlStickButton::getCalculatedActiveZoneSummary()
{
    JoyControlStickModifierButton *tempButton = stick->getModifierButton();
    QString temp;
    QStringList stringlist;

    if (tempButton && tempButton->getButtonState() &&
        tempButton->hasActiveSlots() && getButtonState())
    {
        stringlist.append(tempButton->getCalculatedActiveZoneSummary());
    }

    stringlist.append(JoyButton::getCalculatedActiveZoneSummary());
    temp = stringlist.join(", ");

    return temp;
}

double JoyControlStickButton::getLastMouseDistanceFromDeadZone()
{
    return stick->calculateLastMouseDirectionalDistance(this);
}

double JoyControlStickButton::getCurrentSpringDeadCircle()
{
    double result = (springDeadCircleMultiplier * 0.01);
    if (index == JoyControlStick::StickLeft || index == JoyControlStick::StickRight)
    {
        result = stick->getSpringDeadCircleX() * (springDeadCircleMultiplier * 0.01);
    }
    else if (index == JoyControlStick::StickUp || index == JoyControlStick::StickDown)
    {
        result = stick->getSpringDeadCircleY() * (springDeadCircleMultiplier * 0.01);
    }
    else if (index == JoyControlStick::StickRightUp || index == JoyControlStick::StickRightDown ||
             index == JoyControlStick::StickLeftDown || index == JoyControlStick::StickLeftUp)
    {
        result = 0.0;
    }

    return result;
}
