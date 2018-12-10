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

#include "joyaxis.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "joycontrolstick.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "event.h"

#include <cmath>

#include <QtGlobal>
#include <QDebug>

const JoyAxis::ThrottleTypes JoyAxis::DEFAULTTHROTTLE = JoyAxis::NormalThrottle;

JoyAxis::JoyAxis(int index, int originset, SetJoystick *parentSet,
                 QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_stick = nullptr;
    lastKnownThottledValue = 0;
    lastKnownRawValue = 0;
    axis_max_cal = -1;
    axis_min_cal = -1;
    axis_center_cal = -1;
    m_originset = originset;
    m_parentSet = parentSet;
    naxisbutton = new JoyAxisButton(this, 0, originset, parentSet, this);
    paxisbutton = new JoyAxisButton(this, 1, originset, parentSet, this);

    resetPrivateVars();
    m_index = index;
}

JoyAxis::~JoyAxis()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    resetPrivateVars();
}

void JoyAxis::queuePendingEvent(int value, bool ignoresets, bool updateLastValues)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    pendingEvent = false;
    pendingValue = 0;
    pendingIgnoreSets = false;

    if (m_stick != nullptr)
    {
        stickPassEvent(value, ignoresets, updateLastValues);
    }
    else
    {
        pendingEvent = true;
        pendingValue = value;
        pendingIgnoreSets = ignoresets;
    }
}

void JoyAxis::activatePendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (pendingEvent)
    {
        joyEvent(pendingValue, pendingIgnoreSets);

        pendingEvent = false;
        pendingValue = false;
        pendingIgnoreSets = false;
    }
}

bool JoyAxis::hasPendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return pendingEvent;
}

void JoyAxis::clearPendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    pendingEvent = false;
    pendingValue = false;
    pendingIgnoreSets = false;
}

void JoyAxis::stickPassEvent(int value, bool ignoresets, bool updateLastValues)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
        }
        else if (!safezone && isActive)
        {
            isActive = eventActive = false;
            emit released(value);
        }

        if (!ignoresets) m_stick->queueJoyEvent(ignoresets);
        else m_stick->joyEvent(ignoresets);

        emit moved(currentRawValue);
    }
}

void JoyAxis::joyEvent(int value, bool ignoresets, bool updateLastValues)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((m_stick != nullptr) && !pendingEvent)
    {
        stickPassEvent(value, ignoresets, updateLastValues);
    }
    else
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
        }
        else if (!safezone && isActive)
        {
            isActive = eventActive = false;
            emit released(value);
            createDeskEvent(ignoresets);
        }
        else if (isActive)
        {
            createDeskEvent(ignoresets);
        }
    }

    emit moved(currentRawValue);
}

bool JoyAxis::inDeadZone(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    int temp = calculateThrottledValue(value);

    if (abs(temp) <= deadZone)
    {
        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Value of throttle is in (less than) dead zone: " << abs(temp) << " <= " << deadZone;
        #endif

        result = true;

    } else {

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Value of throttle is not in (greater than) dead zone: " << abs(temp) << " > " << deadZone;
        #endif
    }

    return result;
}

QString JoyAxis::getName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = getPartialName(forceFullFormat, displayNames);
    label.append(": ");

    if (throttle == static_cast<int>(NormalThrottle))
    {
        label.append("-");

        if (!naxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(naxisbutton->getActionName());
        }
        else
        {
            label.append(naxisbutton->getCalculatedActiveZoneSummary());
        }

        label.append(" | +");

        if (!paxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(paxisbutton->getActionName());
        }
        else
        {
            label.append(paxisbutton->getCalculatedActiveZoneSummary());
        }
    }
    else if ((throttle == static_cast<int>(PositiveThrottle)) || (throttle == static_cast<int>(PositiveHalfThrottle)))
    {
        label.append("+");

        if (!paxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(paxisbutton->getActionName());
        }
        else
        {
            label.append(paxisbutton->getCalculatedActiveZoneSummary());
        }
    }
    else if ((throttle == static_cast<int>(NegativeThrottle)) || (throttle == static_cast<int>(NegativeHalfThrottle)))
    {
        label.append("-");

        if (!naxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(naxisbutton->getActionName());
        }
        else
        {
            label.append(naxisbutton->getCalculatedActiveZoneSummary());
        }
    }

    return label;
}

int JoyAxis::getRealJoyIndex()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index + 1;
}

int JoyAxis::getCurrentThrottledValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentThrottledValue;
}

void JoyAxis::updateCurrentThrottledValue(int newValue)
{
    currentThrottledValue = newValue;
}

int JoyAxis::calculateThrottledValue(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Throtted value at start of function is: " << value;
    #endif

    int temp = value;

    switch(throttle)
    {

        case -2:
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It's a negative half throttle";
            #endif

            value = (value <= 0) ? value : -value;
            temp = value;
        break;

        case -1:
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It's a negative throttle";
            #endif

            temp = (value + getAxisMinCal()) / 2;
        break;

        case 1:
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It's a positive throttle";
            #endif

            temp = (value + getAxisMaxCal()) / 2;
        break;

        case 2:
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "It's a positive half throttle";
            #endif

            value = (value >= 0) ? value : -value;
            temp = value;
        break;

    }


    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Calculated value of throttle is: " << temp;
    #endif

    return temp;
}

void JoyAxis::setIndex(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_index = index;
}

int JoyAxis::getIndex()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index;
}


void JoyAxis::createDeskEvent(bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *eventbutton = nullptr;

    if (currentThrottledValue > deadZone) eventbutton = paxisbutton;
    else if (currentThrottledValue < -deadZone) eventbutton = naxisbutton;

    if ((eventbutton != nullptr) && !activeButton)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        eventbutton->joyEvent(eventActive, ignoresets);
        activeButton = eventbutton;
    }
    else if ((eventbutton == nullptr) && (activeButton != nullptr))
    {
        // Currently in deadzone. Disable currently active button.
        activeButton->joyEvent(eventActive, ignoresets);
        activeButton = nullptr;
    }
    else if ((eventbutton != nullptr) && (activeButton != nullptr) && (eventbutton == activeButton))
    {
        //Button is currently active. Just pass current value
        eventbutton->joyEvent(eventActive, ignoresets);
    }
    else if ((eventbutton != nullptr) && (activeButton != nullptr) && (eventbutton != activeButton))
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deadZone = abs(value);
    emit propertyUpdated();
}

int JoyAxis::getDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return deadZone;
}

void JoyAxis::setMaxZoneValue(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    value = abs(value);

    if (value >=getAxisMaxCal())
    {
        maxZoneValue = getAxisMaxCal();
        emit propertyUpdated();
    }
    else
    {
        maxZoneValue = value;
        emit propertyUpdated();
    }
}

int JoyAxis::getMaxZoneValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return maxZoneValue;
}

/**
 * @brief Set throttle value for axis.
 * @param Current value for axis.
 */
void JoyAxis::setThrottle(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Value of throttle for axis in setThrottle is: " << value;
    #endif

    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) && (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        if (value != throttle)
        {
            #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "Throttle value for variable \"throttle\" has been set: " << value;
            #endif

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) && (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle))
            && (value != throttle))
    {
        throttle = value;
        adjustRange();
        emit throttleChanged();
    }
}

int JoyAxis::getThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return throttle;
}

void JoyAxis::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    resetPrivateVars();
}

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

    pendingEvent = false;
    pendingValue = currentRawValue;
    pendingIgnoreSets = false;
}

void JoyAxis::reset(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    reset();
    m_index = index;
}

JoyAxisButton* JoyAxis::getPAxisButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return paxisbutton;
}

JoyAxisButton* JoyAxis::getNAxisButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return naxisbutton;
}

int JoyAxis::getCurrentRawValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentRawValue;
}

void JoyAxis::adjustRange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (throttle == static_cast<int>(JoyAxis::NegativeThrottle))
    {
        currentThrottledDeadValue = getAxisMaxCal();
    }
    else if ((throttle == static_cast<int>(JoyAxis::NormalThrottle)) ||
             (throttle == static_cast<int>(JoyAxis::PositiveHalfThrottle)) ||
             (throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        currentThrottledDeadValue = 0;
    }
    else if (throttle == static_cast<int>(JoyAxis::PositiveThrottle))
    {
        currentThrottledDeadValue = getAxisMinCal();
    }

    currentThrottledValue = calculateThrottledValue(currentRawValue);
}

int JoyAxis::getCurrentThrottledDeadValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentThrottledDeadValue;
}

double JoyAxis::getDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getDistanceFromDeadZone(currentThrottledValue);
}

double JoyAxis::getDistanceFromDeadZone(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double distance = 0.0;

    if (value >= deadZone)
    {
        distance = static_cast<double>(value - deadZone)/(maxZoneValue - deadZone);
    }
    else if (value <= -deadZone)
    {
        distance = static_cast<double>(value + deadZone)/( (-maxZoneValue) + deadZone);
    }

    return qBound(0.0, distance, 1.0);
}

/**
 * @brief Get the current value for an axis in either direction converted to
 *   the range of -1.0 to 1.0.
 * @param Current interger value of the axis
 * @return Axis value in the range of -1.0 to 1.0
 */
double JoyAxis::getRawDistance(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return qBound(-1.0, static_cast<double>(value) / maxZoneValue, 1.0);
}

void JoyAxis::propogateThrottleChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit throttleChangePropogated(m_index);
}

int JoyAxis::getCurrentlyAssignedSet()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_originset;
}

void JoyAxis::setControlStick(JoyControlStick *stick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    removeVDPads();
    removeControlStick();
    m_stick = stick;
    emit propertyUpdated();
}

bool JoyAxis::isPartControlStick()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (m_stick != nullptr);
}

JoyControlStick* JoyAxis::getControlStick()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_stick;
}

void JoyAxis::removeControlStick(bool performRelease)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (m_stick != nullptr)
    {
        if (performRelease)
            m_stick->releaseButtonEvents();

        m_stick = nullptr;
        emit propertyUpdated();
    }
}


bool JoyAxis::hasControlOfButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool value = true;

    if (paxisbutton->isPartVDPad() || naxisbutton->isPartVDPad())
    {
        value = false;
    }

    return value;
}

void JoyAxis::removeVDPads()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= getAxisMinCal()) && (value <= getAxisMaxCal()))
    {
        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Raw value is less than " << getAxisMaxCal() << " and greather than " << getAxisMinCal();
        #endif

        currentRawValue = value;
    }
    else if (value > getAxisMaxCal())
    {
        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Raw value is greather than " << getAxisMaxCal();
        #endif

        currentRawValue = getAxisMaxCal();
    }
    else if (value < getAxisMinCal())
    {
        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Raw value is less than " << getAxisMinCal();
        #endif

        currentRawValue = getAxisMinCal();
    }

    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Raw value for axis is: " << currentRawValue;
    #endif
}

void JoyAxis::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setMouseMode(mode);
    naxisbutton->setMouseMode(mode);
}

bool JoyAxis::hasSameButtonsMouseMode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = true;

    if (paxisbutton->getMouseMode() != naxisbutton->getMouseMode())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyAxis::getButtonsPresetMouseMode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;

    if (paxisbutton->getMouseMode() == naxisbutton->getMouseMode())
        resultMode = paxisbutton->getMouseMode();

    return resultMode;
}

void JoyAxis::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setMouseCurve(mouseCurve);
    naxisbutton->setMouseCurve(mouseCurve);
}

bool JoyAxis::hasSameButtonsMouseCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = true;
    if (paxisbutton->getMouseCurve() != naxisbutton->getMouseCurve())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseCurve JoyAxis::getButtonsPresetMouseCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;
    if (paxisbutton->getMouseCurve() == naxisbutton->getMouseCurve())
    {
        resultCurve = paxisbutton->getMouseCurve();
    }

    return resultCurve;
}

void JoyAxis::setButtonsSpringWidth(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setSpringWidth(value);
    naxisbutton->setSpringWidth(value);
}

void JoyAxis::setButtonsSpringHeight(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setSpringHeight(value);
    naxisbutton->setSpringHeight(value);
}

int JoyAxis::getButtonsPresetSpringWidth()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int presetSpringWidth = 0;

    if (paxisbutton->getSpringWidth() == naxisbutton->getSpringWidth())
        presetSpringWidth = paxisbutton->getSpringWidth();

    return presetSpringWidth;
}

int JoyAxis::getButtonsPresetSpringHeight()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int presetSpringHeight = 0;

    if (paxisbutton->getSpringHeight() == naxisbutton->getSpringHeight())
        presetSpringHeight = paxisbutton->getSpringHeight();

    return presetSpringHeight;
}

void JoyAxis::setButtonsSensitivity(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setSensitivity(value);
    naxisbutton->setSensitivity(value);
}

double JoyAxis::getButtonsPresetSensitivity()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double presetSensitivity = 1.0;

    if (qFuzzyCompare(paxisbutton->getSensitivity(), naxisbutton->getSensitivity()))
        presetSensitivity = paxisbutton->getSensitivity();

    return presetSensitivity;
}

JoyAxisButton* JoyAxis::getAxisButtonByValue(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *eventbutton = nullptr;
    int throttledValue = calculateThrottledValue(value);

    #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "throttledValue in getAxisButtonByValue is: " << throttledValue;
    #endif

    if (throttledValue > deadZone)
    {
        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "throtted value is positive";
        #endif

        eventbutton = paxisbutton;
    }
    else if (throttledValue < (-deadZone))
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "throtted value is negative";
        #endif

        eventbutton = naxisbutton;
    }

    return eventbutton;
}

void JoyAxis::setAxisName(QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((tempName.length() <= 20) && (tempName != axisName))
    {
        axisName = tempName;
        emit axisNameChanged();
        emit propertyUpdated();
    }
}

QString JoyAxis::getAxisName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return axisName;
}

void JoyAxis::setButtonsWheelSpeedX(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setWheelSpeed(value, 'X');
    naxisbutton->setWheelSpeed(value, 'X');
}

void JoyAxis::setButtonsWheelSpeedY(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setWheelSpeed(value, 'Y');
    naxisbutton->setWheelSpeed(value, 'Y');
}

void JoyAxis::setDefaultAxisName(QString tempname)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    defaultAxisName = tempname;
}

QString JoyAxis::getDefaultAxisName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return defaultAxisName;
}

QString JoyAxis::getPartialName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString();

    if (!axisName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            label.append(trUtf8("Axis")).append(" ");
        }

        label.append(axisName);
    }
    else if (!defaultAxisName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(trUtf8("Axis")).append(" ");
        }

        label.append(defaultAxisName);
    }
    else
    {
        label.append(trUtf8("Axis")).append(" ");
        label.append(QString::number(getRealJoyIndex()));
    }

    return label;
}

QString JoyAxis::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyAxis::xmlName;
}

int JoyAxis::getDefaultDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyAxis::AXISDEADZONE;
}

int JoyAxis::getDefaultMaxZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyAxis::AXISMAXZONE;
}

JoyAxis::ThrottleTypes JoyAxis::getDefaultThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->DEFAULTTHROTTLE;
}

SetJoystick* JoyAxis::getParentSet()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_parentSet;
}

void JoyAxis::establishPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(this, &JoyAxis::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyAxis::disconnectPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(this, &JoyAxis::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyAxis::setButtonsSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setSpringRelativeStatus(value);
    naxisbutton->setSpringRelativeStatus(value);
}

bool JoyAxis::isRelativeSpring()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool relative = false;

    if (paxisbutton->isRelativeSpring() == naxisbutton->isRelativeSpring())
        relative = paxisbutton->isRelativeSpring();

    return relative;
}

void JoyAxis::performCalibration(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    InputDevice *device = m_parentSet->getInputDevice();

    if (value <= -30000)
    {
        // Assume axis is a trigger. Set default throttle to Positive.
        device->setCalibrationThrottle(m_index, PositiveThrottle);
    }
    else
    {
        // Ensure that default throttle is used when a device is reset.
        device->setCalibrationThrottle(m_index,
                                       static_cast<JoyAxis::ThrottleTypes>(throttle));
    }
}

void JoyAxis::copyAssignments(JoyAxis *destAxis)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setEasingDuration(value);
    naxisbutton->setEasingDuration(value);
}

double JoyAxis::getButtonsEasingDuration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;

    if (qFuzzyCompare(paxisbutton->getEasingDuration(), naxisbutton->getEasingDuration()))
    {
        result = paxisbutton->getEasingDuration();
    }

    return result;
}

int JoyAxis::getLastKnownThrottleValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return lastKnownThottledValue;
}

int JoyAxis::getLastKnownRawValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return lastKnownRawValue;
}

/**
 * @brief Determine an appropriate release value for an axis depending
 *     on the current throttle setting being used.
 * @return Release value for an axis
 */
int JoyAxis::getProperReleaseValue()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Handles NormalThrottle case
    int value = 0;

    if (throttle == static_cast<int>(NegativeHalfThrottle))
    {
        value = 0;
    }
    else if (throttle == static_cast<int>(NegativeThrottle))
    {
        value = getAxisMaxCal();
    }
    else if (throttle == static_cast<int>(PositiveThrottle))
    {
        value = getAxisMinCal();
    }
    else if (throttle == static_cast<int>(PositiveHalfThrottle))
    {
        value = 0;
    }

    return value;
}

void JoyAxis::setExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    paxisbutton->setExtraAccelerationCurve(curve);
    naxisbutton->setExtraAccelerationCurve(curve);
}

JoyButton::JoyExtraAccelerationCurve JoyAxis::getExtraAccelerationCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyExtraAccelerationCurve result = JoyButton::LinearAccelCurve;

    if (paxisbutton->getExtraAccelerationCurve() == naxisbutton->getExtraAccelerationCurve())
    {
        result = paxisbutton->getExtraAccelerationCurve();
    }

    return result;
}

void JoyAxis::copyRawValues(JoyAxis *srcAxis)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->lastKnownRawValue = srcAxis->lastKnownRawValue;
    this->currentRawValue = srcAxis->currentRawValue;
}

void JoyAxis::copyThrottledValues(JoyAxis *srcAxis)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->lastKnownThottledValue = srcAxis->lastKnownThottledValue;
    this->currentThrottledValue = srcAxis->currentThrottledValue;
}

void JoyAxis::eventReset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    naxisbutton->eventReset();
    paxisbutton->eventReset();
}

void JoyAxis::setAxisMinCal(int value) {

    axis_min_cal = value;
}

int JoyAxis::getAxisMinCal() {

    return ((axis_min_cal != -1) ? axis_min_cal : GlobalVariables::JoyAxis::AXISMIN);
}

void JoyAxis::setAxisMaxCal(int value) {

    axis_max_cal = value;
}

int JoyAxis::getAxisMaxCal() {

    return ((axis_max_cal != -1) ? axis_max_cal : GlobalVariables::JoyAxis::AXISMAX);
}

void JoyAxis::setAxisCenterCal(int value) {

    axis_center_cal = value;
}

int JoyAxis::getAxisCenterCal() {

    return((axis_center_cal != -1) ? axis_center_cal : 0);
}
