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

#include "joycontrolstickbutton.h"

#include "event.h"
#include "globalvariables.h"
#include "joycontrolstick.h"
#include "joycontrolstickmodifierbutton.h"
#include "setjoystick.h"

#include <cmath>

#include <QDebug>
#include <QStringList>

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, int index, int originset, SetJoystick *parentSet,
                                             QObject *parent)
    : JoyGradientButton(index, originset, parentSet, parent)
{
    this->stick = stick;
}

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index,
                                             int originset, SetJoystick *parentSet, QObject *parent)
    : JoyGradientButton(index, originset, parentSet, parent)
{
    this->stick = stick;
}

QString JoyControlStickButton::getDirectionName() const
{
    QString label = QString();

    switch (m_index_sdl)
    {
    case 1:
        label.append(tr("Up"));
        break;

    case 2:
        label.append(tr("Up")).append("+").append(tr("Right"));
        break;

    case 3:
        label.append(tr("Right"));
        break;

    case 4:
        label.append(tr("Down")).append("+").append(tr("Right"));
        break;

    case 5:
        label.append(tr("Down"));
        break;

    case 6:
        label.append(tr("Down")).append("+").append(tr("Left"));
        break;

    case 7:
        label.append(tr("Left"));
        break;

    case 8:
        label.append(tr("Up")).append("+").append(tr("Left"));
        break;
    }

    return label;
}

QString JoyControlStickButton::getPartialName(bool forceFullFormat, bool displayNames) const
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
    } else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(defaultButtonName);
    } else
    {
        temp.append(tr("Button")).append(" ");
        temp.append(getDirectionName());
    }

    return temp;
}

QString JoyControlStickButton::getXmlName() { return GlobalVariables::JoyControlStickButton::xmlName; }

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickButton::getDistanceFromDeadZone() { return stick->calculateDirectionalDistance(); }

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickButton::getMouseDistanceFromDeadZone() { return stick->calculateMouseDirectionalDistance(this); }

void JoyControlStickButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    Q_UNUSED(updateActiveString);
    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(m_index_sdl, this->stick->getIndex(), setSelection, condition);
        } else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(m_index_sdl, this->stick->getIndex(), setSelection, SetChangeDisabled);
        }

        setSelectionCondition = condition;
    } else if (passive)
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

int JoyControlStickButton::getRealJoyNumber() const { return m_index_sdl; }

JoyStickDirectionsType::JoyStickDirections JoyControlStickButton::getDirection() const
{
    return static_cast<JoyStickDirectionsType::JoyStickDirections>(m_index_sdl);
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
bool JoyControlStickButton::isPartRealAxis() { return true; }

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
    QList<JoyButtonSlot *> tempList;

    tempList.append(getActiveZoneList());
    QString temp = buildActiveZoneSummary(tempList);
    return temp;
}

QString JoyControlStickButton::getCalculatedActiveZoneSummary()
{
    JoyControlStickModifierButton *tempButton = stick->getModifierButton();
    QString temp = QString();
    QStringList stringlist = QStringList();

    if ((tempButton != nullptr) && tempButton->getButtonState() && tempButton->hasActiveSlots() && getButtonState())
    {
        qDebug() << "Calculated Active Zone Summary: " << tempButton->getCalculatedActiveZoneSummary();

        stringlist.append(tempButton->getCalculatedActiveZoneSummary());
    }

    stringlist.append(JoyButton::getCalculatedActiveZoneSummary());
    temp = stringlist.join(", ");

    qDebug() << "Returned joined zone: " << temp;

    return temp;
}

double JoyControlStickButton::getLastMouseDistanceFromDeadZone()
{
    return stick->calculateLastMouseDirectionalDistance(this);
}

double JoyControlStickButton::getCurrentSpringDeadCircle()
{
    double result = (springDeadCircleMultiplier * 0.01);

    if ((m_index_sdl == static_cast<int>(JoyControlStick::StickLeft)) ||
        (m_index_sdl == static_cast<int>(JoyControlStick::StickRight)))
    {
        result = stick->getSpringDeadCircleX() * (springDeadCircleMultiplier * 0.01);
    } else if ((m_index_sdl == static_cast<int>(JoyControlStick::StickUp)) ||
               (m_index_sdl == static_cast<int>(JoyControlStick::StickDown)))
    {
        result = stick->getSpringDeadCircleY() * (springDeadCircleMultiplier * 0.01);
    } else if ((m_index_sdl == static_cast<int>(JoyControlStick::StickRightUp)) ||
               (m_index_sdl == static_cast<int>(JoyControlStick::StickRightDown)) ||
               (m_index_sdl == static_cast<int>(JoyControlStick::StickLeftDown)) ||
               (m_index_sdl == static_cast<int>(JoyControlStick::StickLeftUp)))
    {
        result = 0.0;
    }

    return result;
}

JoyControlStick *JoyControlStickButton::getStick() const { return stick; }
