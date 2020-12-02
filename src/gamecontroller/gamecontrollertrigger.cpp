/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.
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

#include "gamecontrollertrigger.h"

#include "gamecontrollertriggerbutton.h"
#include "globalvariables.h"
#include "messagehandler.h"
#include "xml/joyaxisxml.h"

#include <SDL2/SDL_gamecontroller.h>

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const GameControllerTrigger::ThrottleTypes GameControllerTrigger::DEFAULTTHROTTLE =
    GameControllerTrigger::PositiveHalfThrottle;

GameControllerTrigger::GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent)
    : JoyAxis(index, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    naxisbutton = new GameControllerTriggerButton(this, 0, originset, parentSet, this);
    paxisbutton = new GameControllerTriggerButton(this, 1, originset, parentSet, this);
    reset(index);
}

void GameControllerTrigger::reset(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    reset();
    m_index = index;
}

void GameControllerTrigger::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deadZone = GlobalVariables::GameControllerTrigger::AXISDEADZONE;
    isActive = false;

    eventActive = false;
    maxZoneValue = GlobalVariables::GameControllerTrigger::AXISMAXZONE;
    throttle = this->DEFAULTTHROTTLE;

    paxisbutton->reset();
    naxisbutton->reset();
    activeButton = nullptr;
    lastKnownThottledValue = 0;
    lastKnownRawValue = 0;

    adjustRange();
    setCurrentRawValue(currentThrottledDeadValue);
    currentThrottledValue = calculateThrottledValue(currentRawValue);
    axisName.clear();

    pendingEvent = false;
    pendingValue = currentRawValue;
    pendingIgnoreSets = false;
}

QString GameControllerTrigger::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameControllerTrigger::xmlName;
}

QString GameControllerTrigger::getPartialName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString();

    if (!axisName.isEmpty() && displayNames)
    {
        label.append(axisName);

        if (forceFullFormat)
        {
            label.append(" ").append(tr("Trigger"));
        }
    } else if (!defaultAxisName.isEmpty())
    {
        label.append(defaultAxisName);

        if (forceFullFormat)
        {
            label.append(" ").append(tr("Trigger"));
        }
    } else
    {
        label.append(tr("Trigger")).append(" ");
        label.append(QString::number(getRealJoyIndex() - SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    }

    return label;
}

void GameControllerTrigger::correctJoystickThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (this->throttle != static_cast<int>(PositiveHalfThrottle))
    {
        this->setThrottle(static_cast<int>(PositiveHalfThrottle));

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

int GameControllerTrigger::getDefaultDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameControllerTrigger::AXISDEADZONE;
}

int GameControllerTrigger::getDefaultMaxZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameControllerTrigger::AXISMAXZONE;
}

JoyAxis::ThrottleTypes GameControllerTrigger::getDefaultThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return static_cast<ThrottleTypes>(this->DEFAULTTHROTTLE);
}
