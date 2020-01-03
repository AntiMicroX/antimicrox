/* antimicroX Gamepad to KB+M event mapper
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

#include "joycontrolstickmodifierbutton.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "joycontrolstick.h"
#include "setjoystick.h"

#include <cmath>

#include <QDebug>
#include <QString>


JoyControlStickModifierButton::JoyControlStickModifierButton(JoyControlStick *stick, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(0, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->stick = stick;
}


QString JoyControlStickModifierButton::getPartialName(bool forceFullFormat, bool displayNames) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyControlStickModifierButton::xmlName;
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickModifierButton::getDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return stick->calculateDirectionalDistance();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickModifierButton::getMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getDistanceFromDeadZone();
}


void JoyControlStickModifierButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(updateActiveString);
    Q_UNUSED(condition);
    Q_UNUSED(passive);
}


JoyControlStick* JoyControlStickModifierButton::getStick() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return stick;
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyControlStickModifierButton::setTurboMode(TurboMode mode)
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
bool JoyControlStickModifierButton::isPartRealAxis()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return true;
}


bool JoyControlStickModifierButton::isModifierButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return true;
}


double JoyControlStickModifierButton::getAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double temp = stick->getAbsoluteRawDistance();
    return temp;
}


double JoyControlStickModifierButton::getLastAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double temp = stick->calculateLastAccelerationDirectionalDistance();
    return temp;
}


double JoyControlStickModifierButton::getLastMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return stick->calculateLastDirectionalDistance();
}
