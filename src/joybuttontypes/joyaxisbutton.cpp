/* antimicroX Gamepad to KB+M event mapper
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

#include "joyaxisbutton.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "joyaxis.h"
#include "joybutton.h"
#include "setjoystick.h"
#include "vdpad.h"
#include "event.h"

#include <cmath>

#include <QDebug>


JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(index, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_axis = axis;
}


QString JoyAxisButton::getPartialName(bool forceFullFormat, bool displayNames) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString(m_axis->getPartialName(forceFullFormat, displayNames));
    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(defaultButtonName);
    }
    else
    {
        QString buttontype = QString();

        switch(m_index)
        {

            case 0:
            buttontype = tr("Negative");
            break;

            case 1:
            buttontype = tr("Positive");
            break;

            default:
            buttontype = tr("Unknown");
            break;

        }

        temp.append(tr("Button")).append(" ").append(buttontype);
    }

    return temp;
}


QString JoyAxisButton::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyAxisButton::xmlName;
}


void JoyAxisButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(m_index, m_axis->getIndex(), setSelection, condition);
        }
        else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(m_index, m_axis->getIndex(), setSelection, SetChangeDisabled);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_axis->getDistanceFromDeadZone();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyAxisButton::getMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->getDistanceFromDeadZone();
}


void JoyAxisButton::setVDPad(VDPad *vdpad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_axis->isPartControlStick())
    {
        m_axis->removeControlStick();
    }

    JoyButton::setVDPad(vdpad);
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyAxisButton::setTurboMode(TurboMode mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return true;
}


double JoyAxisButton::getAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_axis->getRawDistance(m_axis->getCurrentThrottledValue());
}


double JoyAxisButton::getLastAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_axis->getRawDistance(m_axis->getLastKnownThrottleValue());
}


double JoyAxisButton::getLastMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double distance = 0.0;

    if (m_axis->getAxisButtonByValue(m_axis->getLastKnownThrottleValue()) == this)
    {
        distance = m_axis->getDistanceFromDeadZone(m_axis->getLastKnownThrottleValue());
    }

    return distance;
}


JoyAxis* JoyAxisButton::getAxis() const {

    return m_axis;
}
