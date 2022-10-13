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

#include "joyaxis.h"

#include "event.h"
#include "globalvariables.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "joycontrolstick.h"
#include "setjoystick.h"

#include <cmath>

#include <QDebug>
#include <QScopedPointer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtGlobal>

const JoyAxis::ThrottleTypes JoyAxis::DEFAULTTHROTTLE = JoyAxis::NormalThrottle;

JoyAxis::JoyAxis(int index, int originset, SetJoystick *parentSet, QObject *parent)
    : QObject(parent)
{
    m_stick = nullptr;
    lastKnownThottledValue = 0;
    lastKnownRawValue = 0;
    currentRawValue = 0;
    m_originset = originset;
    m_parentSet = parentSet;
    naxisbutton = new JoyAxisButton(this, 0, originset, parentSet, this);
    paxisbutton = new JoyAxisButton(this, 1, originset, parentSet, this);

    resetPrivateVars();
    m_index = index;
}

JoyAxis::~JoyAxis() { resetPrivateVars(); }

void JoyAxis::queuePendingEvent(int value, bool ignoresets, bool updateLastValues)
{
    if (m_calibrated)
        value = value * m_gain + m_offset;

    if (m_stick != nullptr)
    {
        pendingEvent = false;
        pendingValue = 0;
        pendingIgnoreSets = false;
        stickPassEvent(value, ignoresets, updateLastValues);
    } else
    {
        pendingEvent = true;
        pendingValue = value;
        pendingIgnoreSets = ignoresets;
    }
}

void JoyAxis::activatePendingEvent()
{
    if (pendingEvent)
    {
        joyEvent(pendingValue, pendingIgnoreSets);

        pendingEvent = false;
        pendingValue = false;
        pendingIgnoreSets = false;
    }
}

bool JoyAxis::hasPendingEvent() { return pendingEvent; }

void JoyAxis::clearPendingEvent()
{
    pendingEvent = false;
    pendingValue = false;
    pendingIgnoreSets = false;
}

void JoyAxis::stickPassEvent(int value, bool ignoresets, bool updateLastValues)
{
    if (m_stick != nullptr)
    {
        if (updateLastValues)
        {
            lastKnownThottledValue = currentThrottledValue;
            lastKnownRawValue = currentRawValue;
        }

        setCurrentRawValue(value);

        bool safezone = !inDeadZone(currentRawValue);
        currentThrottledValue = calculateThrottledValue(value);

        if (safezone && !isActive)
        {
            isActive = eventActive = true;
            emit active(value);
        } else if (!safezone && isActive)
        {
            isActive = eventActive = false;
            emit released(value);
        }

        if (!ignoresets)
            m_stick->queueJoyEvent(ignoresets);
        else
            m_stick->joyEvent(ignoresets);

        emit moved(currentRawValue);
    }
}

void JoyAxis::joyEvent(int value, bool ignoresets, bool updateLastValues)
{
    if ((m_stick != nullptr) && !pendingEvent)
    {
        stickPassEvent(value, ignoresets, updateLastValues);
    } else
    {
        if (updateLastValues)
        {
            lastKnownThottledValue = currentThrottledValue;
            lastKnownRawValue = currentRawValue;
        }

        setCurrentRawValue(value);

        bool safezone = !inDeadZone(currentRawValue);
        currentThrottledValue = calculateThrottledValue(value);

        // If in joystick mode and this is the first detected event,
        // use the current value as the axis center point. If the value
        // is below -30,000 then consider it a trigger.
        InputDevice *device = m_parentSet->getInputDevice();

        if (!device->isGameController() && !device->hasCalibrationThrottle(m_index))
        {
            performCalibration(currentRawValue);
            safezone = !inDeadZone(currentRawValue);
            currentThrottledValue = calculateThrottledValue(value);
        }

        if (safezone && !isActive)
        {
            isActive = eventActive = true;
            emit active(value);
            createDeskEvent(ignoresets);
        } else if (!safezone && isActive)
        {
            isActive = eventActive = false;
            emit released(value);
            createDeskEvent(ignoresets);
        } else if (isActive)
        {
            createDeskEvent(ignoresets);
        }
    }

    emit moved(currentRawValue);
}

bool JoyAxis::inDeadZone(int value)
{
    bool result = false;
    int temp = calculateThrottledValue(value);

    if (abs(temp) <= deadZone)
    {
        // qDebug() << "Value of throttle is in (less than) dead zone: " << abs(temp) << " <= " << deadZone;

        result = true;

    } else
    {
        // qDebug() << "Value of throttle is not in (greater than) dead zone: " << abs(temp) << " > " << deadZone;
    }

    return result;
}

QString JoyAxis::getName(bool forceFullFormat, bool displayNames)
{
    QString label = getPartialName(forceFullFormat, displayNames);
    label.append(": ");

    if (throttle == static_cast<int>(NormalThrottle))
    {
        label.append("-");

        if (!naxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(naxisbutton->getActionName());
        } else
        {
            label.append(naxisbutton->getCalculatedActiveZoneSummary());
        }

        label.append(" | +");

        if (!paxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(paxisbutton->getActionName());
        } else
        {
            label.append(paxisbutton->getCalculatedActiveZoneSummary());
        }
    } else if ((throttle == static_cast<int>(PositiveThrottle)) || (throttle == static_cast<int>(PositiveHalfThrottle)))
    {
        label.append("+");

        if (!paxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(paxisbutton->getActionName());
        } else
        {
            label.append(paxisbutton->getCalculatedActiveZoneSummary());
        }
    } else if ((throttle == static_cast<int>(NegativeThrottle)) || (throttle == static_cast<int>(NegativeHalfThrottle)))
    {
        label.append("-");

        if (!naxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(naxisbutton->getActionName());
        } else
        {
            label.append(naxisbutton->getCalculatedActiveZoneSummary());
        }
    }

    return label;
}

int JoyAxis::getRealJoyIndex() { return m_index + 1; }

int JoyAxis::getCurrentThrottledValue() { return currentThrottledValue; }

void JoyAxis::updateCurrentThrottledValue(int newValue) { currentThrottledValue = newValue; }

int JoyAxis::calculateThrottledValue(int value)
{
    // QScopedPointer<LogHelper> log(new DEBUG());
    // *log << "Throttled value at start of function is: " << value;

    int temp = value;

    switch (throttle)
    {
    case -2:
        // *log << ". It's a negative half throttle.";

        value = (value <= 0) ? value : -value;
        temp = value;
        break;

    case -1:
        // *log << ". It's a negative throttle.";

        temp = (value + GlobalVariables::JoyAxis::AXISMIN) / 2;
        break;

    case 1:
        // *log << ". It's a positive throttle.";

        temp = (value + GlobalVariables::JoyAxis::AXISMAX) / 2;
        break;

    case 2:
        // *log << ". It's a positive half throttle.";

        value = (value >= 0) ? value : -value;
        temp = value;
        break;
    }

    // *log << " Calculated value of throttle is: " << temp;

    return temp;
}

/**
 * @brief Checks if the axis supports haptic trigger feedback.
 * @returns True if the axis supports haptic trigger feedback, false otherwise.
 */
bool JoyAxis::hasHapticTrigger() const { return false; }

/**
 * @brief Always returns nullptr for JoyAxis base objects.
 */
HapticTriggerPs5 *JoyAxis::getHapticTrigger() const { return nullptr; }

/**
 * @brief Changes the haptic trigger effect mode.
 * @param[in] mode New haptic trigger effect mode.
 */
void JoyAxis::setHapticTriggerMode(HapticTriggerModePs5) {}

void JoyAxis::setIndex(int index) { m_index = index; }

int JoyAxis::getIndex() { return m_index; }

void JoyAxis::createDeskEvent(bool ignoresets)
{
    JoyAxisButton *eventbutton = nullptr;

    if (currentThrottledValue > deadZone)
        eventbutton = paxisbutton;
    else if (currentThrottledValue < -deadZone)
        eventbutton = naxisbutton;

    if ((eventbutton != nullptr) && !activeButton)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        eventbutton->joyEvent(eventActive, ignoresets);
        activeButton = eventbutton;
    } else if ((eventbutton == nullptr) && (activeButton != nullptr))
    {
        // Currently in deadzone. Disable currently active button.
        activeButton->joyEvent(eventActive, ignoresets);
        activeButton = nullptr;
    } else if ((eventbutton != nullptr) && (activeButton != nullptr) && (eventbutton == activeButton))
    {
        // Button is currently active. Just pass current value
        eventbutton->joyEvent(eventActive, ignoresets);
    } else if ((eventbutton != nullptr) && (activeButton != nullptr) && (eventbutton != activeButton))
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button before enabling
        // the new button
        activeButton->joyEvent(!eventActive, ignoresets);
        eventbutton->joyEvent(eventActive, ignoresets);
        activeButton = eventbutton;
    }
}

void JoyAxis::setDeadZone(int value)
{
    value = abs(value);
    if (deadZone == value)
        return;

    deadZone = value;
    emit propertyUpdated();
    emit hapticTriggerChanged();
}

int JoyAxis::getDeadZone() { return deadZone; }

void JoyAxis::setMaxZoneValue(int value)
{
    value = abs(value);

    if (value >= GlobalVariables::JoyAxis::AXISMAX)
    {
        maxZoneValue = GlobalVariables::JoyAxis::AXISMAX;
        emit propertyUpdated();
    } else
    {
        maxZoneValue = value;
        emit propertyUpdated();
    }
}

int JoyAxis::getMaxZoneValue() { return maxZoneValue; }

/**
 * @brief Set throttle value for axis.
 * @param Current value for axis.
 */
void JoyAxis::setThrottle(int value)
{
    qDebug() << "Value of throttle for axis in setThrottle is: " << value;

    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) &&
        (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        if (value != throttle)
        {
            qDebug() << "Throttle value for variable \"throttle\" has been set: " << value;

            throttle = value;
            adjustRange();
            emit throttleChanged();
            emit propertyUpdated();
        }
    }
}

/**
 * @brief Set the initial calibrated throttle based on the first event
 *     passed by SDL.
 * @param Current value for axis.
 */
void JoyAxis::setInitialThrottle(int value)
{
    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) &&
        (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle)) && (value != throttle))
    {
        throttle = value;
        adjustRange();
        emit throttleChanged();
    }
}

int JoyAxis::getThrottle() { return throttle; }

void JoyAxis::reset() { resetPrivateVars(); }

void JoyAxis::resetPrivateVars()
{
    deadZone = GlobalVariables::JoyAxis::AXISDEADZONE;
    isActive = false;

    eventActive = false;
    maxZoneValue = GlobalVariables::JoyAxis::AXISMAXZONE;
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

    m_calibrated = false;
    pendingEvent = false;
    pendingValue = currentRawValue;
    pendingIgnoreSets = false;
}

void JoyAxis::reset(int index)
{
    reset();
    m_index = index;
}

JoyAxisButton *JoyAxis::getPAxisButton() { return paxisbutton; }

JoyAxisButton *JoyAxis::getNAxisButton() { return naxisbutton; }

int JoyAxis::getCurrentRawValue() { return currentRawValue; }

void JoyAxis::adjustRange()
{
    if (throttle == static_cast<int>(JoyAxis::NegativeThrottle))
    {
        currentThrottledDeadValue = GlobalVariables::JoyAxis::AXISMAX;
    } else if ((throttle == static_cast<int>(JoyAxis::NormalThrottle)) ||
               (throttle == static_cast<int>(JoyAxis::PositiveHalfThrottle)) ||
               (throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        currentThrottledDeadValue = 0;
    } else if (throttle == static_cast<int>(JoyAxis::PositiveThrottle))
    {
        currentThrottledDeadValue = GlobalVariables::JoyAxis::AXISMIN;
    }

    currentThrottledValue = calculateThrottledValue(currentRawValue);
}

int JoyAxis::getCurrentThrottledDeadValue() { return currentThrottledDeadValue; }

double JoyAxis::getDistanceFromDeadZone() { return getDistanceFromDeadZone(currentThrottledValue); }

double JoyAxis::getDistanceFromDeadZone(int value)
{
    double distance = 0.0;

    if (value >= deadZone)
    {
        distance = static_cast<double>(value - deadZone) / (maxZoneValue - deadZone);
    } else if (value <= -deadZone)
    {
        distance = static_cast<double>(value + deadZone) / ((-maxZoneValue) + deadZone);
    }

    return qBound(0.0, distance, 1.0);
}

/**
 * @brief Get the current value for an axis in either direction converted to
 *   the range of -1.0 to 1.0.
 * @param Current interger value of the axis
 * @return Axis value in the range of -1.0 to 1.0
 */
double JoyAxis::getRawDistance(int value) { return qBound(-1.0, static_cast<double>(value) / maxZoneValue, 1.0); }

void JoyAxis::propogateThrottleChange() { emit throttleChangePropogated(m_index); }

int JoyAxis::getCurrentlyAssignedSet() { return m_originset; }

void JoyAxis::setControlStick(JoyControlStick *stick)
{
    removeVDPads();
    removeControlStick();
    m_stick = stick;
    emit propertyUpdated();
}

bool JoyAxis::isPartControlStick() { return (m_stick != nullptr); }

JoyControlStick *JoyAxis::getControlStick() { return m_stick; }

void JoyAxis::removeControlStick(bool performRelease)
{
    if (m_stick != nullptr)
    {
        if (performRelease)
            m_stick->releaseButtonEvents();

        m_stick = nullptr;
        // emit propertyUpdated();
    }
}

bool JoyAxis::hasControlOfButtons()
{
    bool value = true;

    if (paxisbutton->isPartVDPad() || naxisbutton->isPartVDPad())
    {
        value = false;
    }

    return value;
}

void JoyAxis::removeVDPads()
{
    if (paxisbutton->isPartVDPad())
    {
        paxisbutton->joyEvent(false, true);
        paxisbutton->removeVDPad();
    }

    if (naxisbutton->isPartVDPad())
    {
        naxisbutton->joyEvent(false, true);
        naxisbutton->removeVDPad();
    }
}

bool JoyAxis::isDefault()
{
    bool value = true;
    value = value && (deadZone == getDefaultDeadZone());
    value = value && (maxZoneValue == getDefaultMaxZone());
    value = value && (paxisbutton->isDefault());
    value = value && (naxisbutton->isDefault());

    return value;
}

/* Use this method to keep currentRawValue in the expected range.
 * SDL has a minimum axis value of -32768 which should be ignored to
 * ensure that JoyControlStick will not encounter overflow problems
 * on a 32 bit machine.
 */
void JoyAxis::setCurrentRawValue(int value)
{
    currentRawValue = qBound(GlobalVariables::JoyAxis::AXISMIN, value, GlobalVariables::JoyAxis::AXISMAX);
}

void JoyAxis::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    paxisbutton->setMouseMode(mode);
    naxisbutton->setMouseMode(mode);
}

bool JoyAxis::hasSameButtonsMouseMode()
{
    bool result = true;

    if (paxisbutton->getMouseMode() != naxisbutton->getMouseMode())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyAxis::getButtonsPresetMouseMode()
{
    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;

    if (paxisbutton->getMouseMode() == naxisbutton->getMouseMode())
        resultMode = paxisbutton->getMouseMode();

    return resultMode;
}

void JoyAxis::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    paxisbutton->setMouseCurve(mouseCurve);
    naxisbutton->setMouseCurve(mouseCurve);
}

bool JoyAxis::hasSameButtonsMouseCurve()
{
    bool result = true;
    if (paxisbutton->getMouseCurve() != naxisbutton->getMouseCurve())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseCurve JoyAxis::getButtonsPresetMouseCurve()
{
    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;
    if (paxisbutton->getMouseCurve() == naxisbutton->getMouseCurve())
    {
        resultCurve = paxisbutton->getMouseCurve();
    }

    return resultCurve;
}

void JoyAxis::setButtonsSpringWidth(int value)
{
    paxisbutton->setSpringWidth(value);
    naxisbutton->setSpringWidth(value);
}

void JoyAxis::setButtonsSpringHeight(int value)
{
    paxisbutton->setSpringHeight(value);
    naxisbutton->setSpringHeight(value);
}

int JoyAxis::getButtonsPresetSpringWidth()
{
    int presetSpringWidth = 0;

    if (paxisbutton->getSpringWidth() == naxisbutton->getSpringWidth())
        presetSpringWidth = paxisbutton->getSpringWidth();

    return presetSpringWidth;
}

int JoyAxis::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    if (paxisbutton->getSpringHeight() == naxisbutton->getSpringHeight())
        presetSpringHeight = paxisbutton->getSpringHeight();

    return presetSpringHeight;
}

void JoyAxis::setButtonsSensitivity(double value)
{
    paxisbutton->setSensitivity(value);
    naxisbutton->setSensitivity(value);
}

double JoyAxis::getButtonsPresetSensitivity()
{
    double presetSensitivity = 1.0;

    if (qFuzzyCompare(paxisbutton->getSensitivity(), naxisbutton->getSensitivity()))
        presetSensitivity = paxisbutton->getSensitivity();

    return presetSensitivity;
}

JoyAxisButton *JoyAxis::getAxisButtonByValue(int value)
{
    JoyAxisButton *eventbutton = nullptr;
    int throttledValue = calculateThrottledValue(value);

    qDebug() << "throttledValue in getAxisButtonByValue is: " << throttledValue;

    if (throttledValue > deadZone)
    {
        qDebug() << "throtted value is positive";

        eventbutton = paxisbutton;
    } else if (throttledValue < (-deadZone))
    {
        qDebug() << "throtted value is negative";

        eventbutton = naxisbutton;
    }

    return eventbutton;
}

void JoyAxis::setAxisName(QString tempName)
{
    if ((tempName.length() <= 20) && (tempName != axisName))
    {
        axisName = tempName;
        emit axisNameChanged();
        emit propertyUpdated();
    }
}

QString JoyAxis::getAxisName() { return axisName; }

void JoyAxis::setButtonsWheelSpeedX(int value)
{
    paxisbutton->setWheelSpeed(value, 'X');
    naxisbutton->setWheelSpeed(value, 'X');
}

void JoyAxis::setButtonsWheelSpeedY(int value)
{
    paxisbutton->setWheelSpeed(value, 'Y');
    naxisbutton->setWheelSpeed(value, 'Y');
}

void JoyAxis::setDefaultAxisName(QString tempname) { defaultAxisName = tempname; }

QString JoyAxis::getDefaultAxisName() { return defaultAxisName; }

QString JoyAxis::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString label = QString();

    if (!axisName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            label.append(tr("Axis")).append(" ");
        }

        label.append(axisName);
    } else if (!defaultAxisName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("Axis")).append(" ");
        }

        label.append(defaultAxisName);
    } else
    {
        label.append(tr("Axis")).append(" ");
        label.append(QString::number(getRealJoyIndex()));
    }

    return label;
}

QString JoyAxis::getXmlName() { return GlobalVariables::JoyAxis::xmlName; }

int JoyAxis::getDefaultDeadZone() { return GlobalVariables::JoyAxis::AXISDEADZONE; }

int JoyAxis::getDefaultMaxZone() { return GlobalVariables::JoyAxis::AXISMAXZONE; }

JoyAxis::ThrottleTypes JoyAxis::getDefaultThrottle() { return this->DEFAULTTHROTTLE; }

SetJoystick *JoyAxis::getParentSet() { return m_parentSet; }

void JoyAxis::establishPropertyUpdatedConnection()
{
    connect(this, &JoyAxis::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyAxis::disconnectPropertyUpdatedConnection()
{
    disconnect(this, &JoyAxis::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyAxis::setButtonsSpringRelativeStatus(bool value)
{
    paxisbutton->setSpringRelativeStatus(value);
    naxisbutton->setSpringRelativeStatus(value);
}

bool JoyAxis::isRelativeSpring()
{
    bool relative = false;

    if (paxisbutton->isRelativeSpring() == naxisbutton->isRelativeSpring())
        relative = paxisbutton->isRelativeSpring();

    return relative;
}

void JoyAxis::performCalibration(int value)
{
    InputDevice *device = m_parentSet->getInputDevice();

    if (value <= -30000)
    {
        // Assume axis is a trigger. Set default throttle to Positive.
        device->setCalibrationThrottle(m_index, PositiveThrottle);
    } else
    {
        // Ensure that default throttle is used when a device is reset.
        device->setCalibrationThrottle(m_index, static_cast<JoyAxis::ThrottleTypes>(throttle));
    }
}

void JoyAxis::copyAssignments(JoyAxis *destAxis)
{
    destAxis->reset();
    destAxis->deadZone = deadZone;
    destAxis->maxZoneValue = maxZoneValue;
    destAxis->axisName = axisName;
    paxisbutton->copyAssignments(destAxis->paxisbutton);
    naxisbutton->copyAssignments(destAxis->naxisbutton);

    if (!destAxis->isDefault())
        emit propertyUpdated();
}

void JoyAxis::setButtonsEasingDuration(double value)
{
    paxisbutton->setEasingDuration(value);
    naxisbutton->setEasingDuration(value);
}

double JoyAxis::getButtonsEasingDuration()
{
    double result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;

    if (qFuzzyCompare(paxisbutton->getEasingDuration(), naxisbutton->getEasingDuration()))
    {
        result = paxisbutton->getEasingDuration();
    }

    return result;
}

int JoyAxis::getLastKnownThrottleValue() { return lastKnownThottledValue; }

int JoyAxis::getLastKnownRawValue() { return lastKnownRawValue; }

/**
 * @brief Determine an appropriate release value for an axis depending
 *     on the current throttle setting being used.
 * @return Release value for an axis
 */
int JoyAxis::getProperReleaseValue()
{ // Handles NormalThrottle case
    int value = 0;

    if (throttle == static_cast<int>(NegativeHalfThrottle))
    {
        value = 0;
    } else if (throttle == static_cast<int>(NegativeThrottle))
    {
        value = GlobalVariables::JoyAxis::AXISMAX;
    } else if (throttle == static_cast<int>(PositiveThrottle))
    {
        value = GlobalVariables::JoyAxis::AXISMIN;
    } else if (throttle == static_cast<int>(PositiveHalfThrottle))
    {
        value = 0;
    }

    return value;
}

void JoyAxis::setExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve)
{
    paxisbutton->setExtraAccelerationCurve(curve);
    naxisbutton->setExtraAccelerationCurve(curve);
}

JoyButton::JoyExtraAccelerationCurve JoyAxis::getExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve result = JoyButton::LinearAccelCurve;

    if (paxisbutton->getExtraAccelerationCurve() == naxisbutton->getExtraAccelerationCurve())
    {
        result = paxisbutton->getExtraAccelerationCurve();
    }

    return result;
}

void JoyAxis::copyRawValues(JoyAxis *srcAxis)
{
    this->lastKnownRawValue = srcAxis->lastKnownRawValue;
    this->currentRawValue = srcAxis->currentRawValue;
}

void JoyAxis::copyThrottledValues(JoyAxis *srcAxis)
{
    this->lastKnownThottledValue = srcAxis->lastKnownThottledValue;
    this->currentThrottledValue = srcAxis->currentThrottledValue;
}

void JoyAxis::eventReset()
{
    naxisbutton->eventReset();
    paxisbutton->eventReset();
}

/**
 * @brief Check if the axis is calibrated
 * @returns True if the axis is calibrated, false otherwise.
 */
bool JoyAxis::isCalibrated() const { return m_calibrated; }

/**
 * @brief Resets the axis calibration back to uncalibrated state.
 */
void JoyAxis::resetCalibration() { m_calibrated = false; }

/**
 * @brief Reads the axis calibration values
 * @param[out] offset Offset value "a"
 * @param[out] gain Gain value "b"
 */
void JoyAxis::getCalibration(double *offset, double *gain) const
{
    *offset = m_offset;
    *gain = m_gain;
}

/**
 * @brief Sets the axis calibration values and sets the calibration flag.
 *  Calibrated value is calculated by the formula "a+b*x".
 */
void JoyAxis::setCalibration(double offset, double gain)
{
    m_calibrated = true;
    m_offset = offset;
    m_gain = gain;
}
