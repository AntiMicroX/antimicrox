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
#include "haptictriggerps5.h"
#include "inputdevice.h"
#include "setjoystick.h"
#include "xml/joyaxisxml.h"

#include <SDL2/SDL_gamecontroller.h>

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const GameControllerTrigger::ThrottleTypes GameControllerTrigger::DEFAULTTHROTTLE =
    GameControllerTrigger::PositiveHalfThrottle;

GameControllerTrigger::GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent)
    : JoyAxis(index, originset, parentSet, parent)
    , m_haptic_trigger(0)
{
    naxisbutton->deleteLater();
    naxisbutton = new GameControllerTriggerButton(this, 0, originset, parentSet, this);
    paxisbutton->deleteLater();
    paxisbutton = new GameControllerTriggerButton(this, 1, originset, parentSet, this);
    reset(index);

#if SDL_VERSION_ATLEAST(2, 0, 16)
    if (parentSet->getInputDevice()->getControllerType() == SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS5)
        m_haptic_trigger = new HapticTriggerPs5(this);
#endif
}

void GameControllerTrigger::reset(int index)
{
    reset();
    m_index = index;
}

void GameControllerTrigger::reset()
{
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

QString GameControllerTrigger::getXmlName() { return GlobalVariables::GameControllerTrigger::xmlName; }

QString GameControllerTrigger::getPartialName(bool forceFullFormat, bool displayNames)
{
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
    if (this->throttle != static_cast<int>(PositiveHalfThrottle))
    {
        this->setThrottle(static_cast<int>(PositiveHalfThrottle));

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

int GameControllerTrigger::getDefaultDeadZone() { return GlobalVariables::GameControllerTrigger::AXISDEADZONE; }

int GameControllerTrigger::getDefaultMaxZone() { return GlobalVariables::GameControllerTrigger::AXISMAXZONE; }

JoyAxis::ThrottleTypes GameControllerTrigger::getDefaultThrottle()
{
    return static_cast<ThrottleTypes>(this->DEFAULTTHROTTLE);
}

/**
 * @brief Checks if the trigger supports haptic feedback.
 * @returns True if the trigger supports haptic feedback, false otherwise.
 */
bool GameControllerTrigger::hasHapticTrigger() const { return m_haptic_trigger != 0; }

/**
 * @brief Recalculates haptic trigger effect positions, e.g. after dead zone change,
 *   and returns the current HapticTriggerPs5 object.
 * @returns Pointer to HapticTriggerPs5 object of this trigger.
 */
HapticTriggerPs5 *GameControllerTrigger::getHapticTrigger() const
{
    if (m_haptic_trigger == nullptr)
        return nullptr;

    int start, end;
    switch (m_haptic_trigger->get_mode())
    {
    case HAPTIC_TRIGGER_NONE:
        m_haptic_trigger->set_effect(0, 0, 0);
        break;
    case HAPTIC_TRIGGER_CLICK:
        end = std::max(160, 200 * deadZone / GlobalVariables::JoyAxis::AXISMAX + 60);
        start = end - 97;
        m_haptic_trigger->set_effect(GlobalVariables::HapticTriggerPs5::CLICKSTRENGTH, start, end);
        break;
    case HAPTIC_TRIGGER_RIGID:
        m_haptic_trigger->set_effect(GlobalVariables::HapticTriggerPs5::RIGIDSTRENGTH, 0,
                                     GlobalVariables::HapticTriggerPs5::RANGE);
        break;
    case HAPTIC_TRIGGER_RIGID_GRADIENT:
        m_haptic_trigger->set_effect(GlobalVariables::HapticTriggerPs5::RIGIDGRADIENTSTRENGTH, 0,
                                     GlobalVariables::HapticTriggerPs5::RANGE);
        break;
    case HAPTIC_TRIGGER_VIBRATION:
        start = 200 * deadZone / GlobalVariables::JoyAxis::AXISMAX;
        m_haptic_trigger->set_effect(GlobalVariables::HapticTriggerPs5::VIBRATIONSTRENGTH, start,
                                     GlobalVariables::HapticTriggerPs5::RANGE, GlobalVariables::HapticTriggerPs5::FREQUENCY);
        break;
    }
    return m_haptic_trigger;
}

/**
 * @brief Changes the haptic feedback effect mode.
 * @param[in] mode New haptic feedback effect mode.
 */
void GameControllerTrigger::setHapticTriggerMode(HapticTriggerModePs5 mode)
{
    if (m_haptic_trigger == 0)
        return;

    if (m_haptic_trigger->set_effect_mode(mode))
        emit hapticTriggerChanged();
}
