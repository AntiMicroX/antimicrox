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

#include "joyaxis.h"
#include "joycontrolstick.h"
#include "inputdevice.h"
#include "event.h"

// Set default values for many properties.
const int JoyAxis::AXISMIN = -32767;
const int JoyAxis::AXISMAX = 32767;
const int JoyAxis::AXISDEADZONE = 6000;
const int JoyAxis::AXISMAXZONE = 32000;

// Speed in pixels/second
const float JoyAxis::JOYSPEED = 20.0;

const JoyAxis::ThrottleTypes JoyAxis::DEFAULTTHROTTLE = JoyAxis::NormalThrottle;

const QString JoyAxis::xmlName = "axis";

JoyAxis::JoyAxis(int index, int originset, SetJoystick *parentSet,
                 QObject *parent) :
    QObject(parent)
{
    stick = 0;
    lastKnownThottledValue = 0;
    lastKnownRawValue = 0;
    this->originset = originset;
    this->parentSet = parentSet;
    naxisbutton = new JoyAxisButton(this, 0, originset, parentSet, this);
    paxisbutton = new JoyAxisButton(this, 1, originset, parentSet, this);

    reset();
    this->index = index;
}

JoyAxis::~JoyAxis()
{
    reset();
}

void JoyAxis::queuePendingEvent(int value, bool ignoresets, bool updateLastValues)
{
    pendingEvent = false;
    pendingValue = 0;
    pendingIgnoreSets = false;
    //pendingUpdateLastValues = true;

    if (this->stick)
    {
        stickPassEvent(value, ignoresets, updateLastValues);
    }
    else
    {
        pendingEvent = true;
        pendingValue = value;
        pendingIgnoreSets = ignoresets;
        //pendingUpdateLastValues = updateLastValues;
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
        //pendingUpdateLastValues = true;
    }
}

bool JoyAxis::hasPendingEvent()
{
    return pendingEvent;
}

void JoyAxis::clearPendingEvent()
{
    pendingEvent = false;
    pendingValue = false;
    pendingIgnoreSets = false;
}

void JoyAxis::stickPassEvent(int value, bool ignoresets, bool updateLastValues)
{
    if (this->stick)
    {
        if (updateLastValues)
        {
            lastKnownThottledValue = currentThrottledValue;
            lastKnownRawValue = currentRawValue;
        }

        setCurrentRawValue(value);
        //currentRawValue = value;
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

        if (!ignoresets)
        {
            stick->queueJoyEvent(ignoresets);
        }
        else
        {
            stick->joyEvent(ignoresets);
        }

        emit moved(currentRawValue);
    }
}

void JoyAxis::joyEvent(int value, bool ignoresets, bool updateLastValues)
{
    if (this->stick && !pendingEvent)
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
        //currentRawValue = value;
        bool safezone = !inDeadZone(currentRawValue);
        currentThrottledValue = calculateThrottledValue(value);

        // If in joystick mode and this is the first detected event,
        // use the current value as the axis center point. If the value
        // is below -30,000 then consider it a trigger.
        InputDevice *device = parentSet->getInputDevice();
        if (!device->isGameController() && !device->hasCalibrationThrottle(index))
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
    bool result = false;
    int temp = calculateThrottledValue(value);

    if (abs(temp) <= deadZone)
    {
        result = true;
    }

    return result;
}

QString JoyAxis::getName(bool forceFullFormat, bool displayNames)
{
    QString label = getPartialName(forceFullFormat, displayNames);

    label.append(": ");

    if (throttle == NormalThrottle)
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
    else if (throttle == PositiveThrottle || throttle == PositiveHalfThrottle)
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
    else if (throttle == NegativeThrottle || throttle == NegativeHalfThrottle)
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
    return index + 1;
}

int JoyAxis::getCurrentThrottledValue()
{
    return currentThrottledValue;
}

int JoyAxis::calculateThrottledValue(int value)
{
    int temp = value;

    if (throttle == NegativeHalfThrottle)
    {
        value = value <= 0 ? value : -value;
        temp = value;
    }
    else if (throttle == NegativeThrottle)
    {
        temp = (value + AXISMIN) / 2;
    }
    else if (throttle == PositiveThrottle)
    {
        temp = (value + AXISMAX) / 2;
    }
    else if (throttle == PositiveHalfThrottle)
    {
        value = value >= 0 ? value : -value;
        temp = value;
    }

    return temp;
}

void JoyAxis::setIndex(int index)
{
    this->index = index;
}

int JoyAxis::getIndex()
{
    return index;
}


void JoyAxis::createDeskEvent(bool ignoresets)
{
    JoyAxisButton *eventbutton = 0;
    if (currentThrottledValue > deadZone)
    {
        eventbutton = paxisbutton;
    }
    else if (currentThrottledValue < -deadZone)
    {
        eventbutton = naxisbutton;
    }

    if (eventbutton && !activeButton)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        eventbutton->joyEvent(eventActive, ignoresets);
        activeButton = eventbutton;
    }
    else if (!eventbutton && activeButton)
    {
        // Currently in deadzone. Disable currently active button.
        activeButton->joyEvent(eventActive, ignoresets);
        activeButton = 0;
    }
    else if (eventbutton && activeButton && eventbutton == activeButton)
    {
        //Button is currently active. Just pass current value
        eventbutton->joyEvent(eventActive, ignoresets);
    }
    else if (eventbutton && activeButton && eventbutton != activeButton)
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
    deadZone = abs(value);
    emit propertyUpdated();
}

int JoyAxis::getDeadZone()
{
    return deadZone;
}

void JoyAxis::setMaxZoneValue(int value)
{
    value = abs(value);
    if (value >= AXISMAX)
    {
        maxZoneValue = AXISMAX;
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
    return maxZoneValue;
}

/**
 * @brief Set throttle value for axis.
 * @param Current value for axis.
 */
void JoyAxis::setThrottle(int value)
{
    if (value >= JoyAxis::NegativeHalfThrottle && value <= JoyAxis::PositiveHalfThrottle)
    {
        if (value != throttle)
        {
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
    if (value >= JoyAxis::NegativeHalfThrottle && value <= JoyAxis::PositiveHalfThrottle)
    {
        if (value != throttle)
        {
            throttle = value;
            adjustRange();
            emit throttleChanged();
        }
    }
}

int JoyAxis::getThrottle()
{
    return throttle;
}

void JoyAxis::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            bool found = false;
            found = readMainConfig(xml);
            if (!found && xml->name() == naxisbutton->getXmlName() && xml->isStartElement())
            {
                found = true;
                readButtonConfig(xml);
            }

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyAxis::writeConfig(QXmlStreamWriter *xml)
{
    bool currentlyDefault = isDefault();

    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number(index+1));

    if (!currentlyDefault)
    {
        if (deadZone != AXISDEADZONE)
        {
            xml->writeTextElement("deadZone", QString::number(deadZone));
        }

        if (maxZoneValue != AXISMAXZONE)
        {
            xml->writeTextElement("maxZone", QString::number(maxZoneValue));
        }
    }


    //if (throttle != DEFAULTTHROTTLE)
    //{
        xml->writeStartElement("throttle");

        if (throttle == JoyAxis::NegativeHalfThrottle)
        {
            xml->writeCharacters("negativehalf");
        }
        else if (throttle == JoyAxis::NegativeThrottle)
        {
            xml->writeCharacters("negative");
        }
        else if (throttle == JoyAxis::NormalThrottle)
        {
            xml->writeCharacters("normal");
        }
        else if (throttle == JoyAxis::PositiveThrottle)
        {
            xml->writeCharacters("positive");
        }
        else if (throttle == JoyAxis::PositiveHalfThrottle)
        {
            xml->writeCharacters("positivehalf");
        }

        xml->writeEndElement();
    //}

    if (!currentlyDefault)
    {
        naxisbutton->writeConfig(xml);
        paxisbutton->writeConfig(xml);
    }

    xml->writeEndElement();
}


bool JoyAxis::readMainConfig(QXmlStreamReader *xml)
{
    bool found = false;

    if (xml->name() == "deadZone" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setDeadZone(tempchoice);
    }
    else if (xml->name() == "maxZone" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setMaxZoneValue(tempchoice);
    }
    else if (xml->name() == "throttle" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "negativehalf")
        {
            this->setThrottle(JoyAxis::NegativeHalfThrottle);
        }
        else if (temptext == "negative")
        {
            this->setThrottle(JoyAxis::NegativeThrottle);
        }
        else if (temptext == "normal")
        {
            this->setThrottle(JoyAxis::NormalThrottle);
        }
        else if (temptext == "positive")
        {
            this->setThrottle(JoyAxis::PositiveThrottle);
        }
        else if (temptext == "positivehalf")
        {
            this->setThrottle(JoyAxis::PositiveHalfThrottle);
        }

        InputDevice *device = parentSet->getInputDevice();
        if (!device->hasCalibrationThrottle(index))
        {
            device->setCalibrationStatus(index,
                                         static_cast<JoyAxis::ThrottleTypes>(throttle));
        }

        setCurrentRawValue(currentThrottledDeadValue);
        //currentRawValue = currentThrottledDeadValue;
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }

    return found;
}

bool JoyAxis::readButtonConfig(QXmlStreamReader *xml)
{
    bool found = false;

    int index = xml->attributes().value("index").toString().toInt();
    if (index == 1)
    {
        found = true;
        naxisbutton->readConfig(xml);
    }
    else if (index == 2)
    {
        found = true;
        paxisbutton->readConfig(xml);
    }

    return found;
}

void JoyAxis::reset()
{
    deadZone = getDefaultDeadZone();
    isActive = false;

    eventActive = false;
    maxZoneValue = getDefaultMaxZone();
    throttle = getDefaultThrottle();

    paxisbutton->reset();
    naxisbutton->reset();
    activeButton = 0;
    lastKnownThottledValue = 0;
    lastKnownRawValue = 0;

    adjustRange();
    setCurrentRawValue(currentThrottledDeadValue);
    currentThrottledValue = calculateThrottledValue(currentRawValue);
    axisName.clear();

    pendingEvent = false;
    pendingValue = currentRawValue;
    pendingIgnoreSets = false;
    //pendingUpdateLastValues = true;
}

void JoyAxis::reset(int index)
{
    reset();
    this->index = index;
}

JoyAxisButton* JoyAxis::getPAxisButton()
{
    return paxisbutton;
}

JoyAxisButton* JoyAxis::getNAxisButton()
{
    return naxisbutton;
}

int JoyAxis::getCurrentRawValue()
{
    return currentRawValue;
}

void JoyAxis::adjustRange()
{
    if (throttle == JoyAxis::NegativeThrottle)
    {
        currentThrottledDeadValue = AXISMAX;
    }
    else if (throttle == JoyAxis::NormalThrottle ||
             throttle == JoyAxis::PositiveHalfThrottle ||
             throttle == JoyAxis::NegativeHalfThrottle)
    {
        currentThrottledDeadValue = 0;
    }
    else if (throttle == JoyAxis::PositiveThrottle)
    {
        currentThrottledDeadValue = AXISMIN;
    }

    currentThrottledValue = calculateThrottledValue(currentRawValue);
}

int JoyAxis::getCurrentThrottledDeadValue()
{
    return currentThrottledDeadValue;
}

double JoyAxis::getDistanceFromDeadZone()
{
    return getDistanceFromDeadZone(currentThrottledValue);
}

double JoyAxis::getDistanceFromDeadZone(int value)
{
    double distance = 0.0;
    int currentValue = value;

    if (currentValue >= deadZone)
    {
        distance = (currentValue - deadZone)/static_cast<double>(maxZoneValue - deadZone);
    }
    else if (currentValue <= -deadZone)
    {
        distance = (currentValue + deadZone)/static_cast<double>(-maxZoneValue + deadZone);
    }

    distance = qBound(0.0, distance, 1.0);
    return distance;
}

/**
 * @brief Get the current value for an axis in either direction converted to
 *   the range of -1.0 to 1.0.
 * @param Current interger value of the axis
 * @return Axis value in the range of -1.0 to 1.0
 */
double JoyAxis::getRawDistance(int value)
{
    double distance = 0.0;
    int currentValue = value;

    distance = currentValue / static_cast<double>(maxZoneValue);
    distance = qBound(-1.0, distance, 1.0);

    return distance;
}

void JoyAxis::propogateThrottleChange()
{
    emit throttleChangePropogated(this->index);
}

int JoyAxis::getCurrentlyAssignedSet()
{
    return originset;
}

void JoyAxis::setControlStick(JoyControlStick *stick)
{
    removeVDPads();
    removeControlStick();
    this->stick = stick;
    emit propertyUpdated();
}

bool JoyAxis::isPartControlStick()
{
    return (this->stick != 0);
}

JoyControlStick* JoyAxis::getControlStick()
{
    return this->stick;
}

void JoyAxis::removeControlStick(bool performRelease)
{
    if (stick)
    {
        if (performRelease)
        {
            stick->releaseButtonEvents();
        }

        this->stick = 0;
        emit propertyUpdated();
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
    //value = value && (throttle == getDefaultThrottle());
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
    if (value >= JoyAxis::AXISMIN && value <= JoyAxis::AXISMAX)
    {
        currentRawValue = value;
    }
    else if (value > JoyAxis::AXISMAX)
    {
        currentRawValue = JoyAxis::AXISMAX;
    }
    else if (value < JoyAxis::AXISMIN)
    {
        currentRawValue = JoyAxis::AXISMIN;
    }
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
    {
        resultMode = paxisbutton->getMouseMode();
    }

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
    {
        presetSpringWidth = paxisbutton->getSpringWidth();
    }

    return presetSpringWidth;
}

int JoyAxis::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    if (paxisbutton->getSpringHeight() == naxisbutton->getSpringHeight())
    {
        presetSpringHeight = paxisbutton->getSpringHeight();
    }

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

    if (paxisbutton->getSensitivity() == naxisbutton->getSensitivity())
    {
        presetSensitivity = paxisbutton->getSensitivity();
    }

    return presetSensitivity;
}

JoyAxisButton* JoyAxis::getAxisButtonByValue(int value)
{
    JoyAxisButton *eventbutton = 0;
    int throttledValue = calculateThrottledValue(value);
    if (throttledValue > deadZone)
    {
        eventbutton = paxisbutton;
    }
    else if (throttledValue < -deadZone)
    {
        eventbutton = naxisbutton;
    }

    return eventbutton;
}

void JoyAxis::setAxisName(QString tempName)
{
    if (tempName.length() <= 20 && tempName != axisName)
    {
        axisName = tempName;
        emit axisNameChanged();
        emit propertyUpdated();
    }
}

QString JoyAxis::getAxisName()
{
    return axisName;
}

void JoyAxis::setButtonsWheelSpeedX(int value)
{
    paxisbutton->setWheelSpeedX(value);
    naxisbutton->setWheelSpeedX(value);
}

void JoyAxis::setButtonsWheelSpeedY(int value)
{
    paxisbutton->setWheelSpeedY(value);
    naxisbutton->setWheelSpeedY(value);
}

void JoyAxis::setDefaultAxisName(QString tempname)
{
    defaultAxisName = tempname;
}

QString JoyAxis::getDefaultAxisName()
{
    return defaultAxisName;
}

QString JoyAxis::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString label;

    if (!axisName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            label.append(tr("Axis")).append(" ");
        }

        label.append(axisName);
    }
    else if (!defaultAxisName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("Axis")).append(" ");
        }
        label.append(defaultAxisName);
    }
    else
    {
        label.append(tr("Axis")).append(" ");
        label.append(QString::number(getRealJoyIndex()));
    }

    return label;
}

QString JoyAxis::getXmlName()
{
    return this->xmlName;
}

int JoyAxis::getDefaultDeadZone()
{
    return this->AXISDEADZONE;
}

int JoyAxis::getDefaultMaxZone()
{
    return this->AXISMAXZONE;
}

JoyAxis::ThrottleTypes JoyAxis::getDefaultThrottle()
{
    return this->DEFAULTTHROTTLE;
}

SetJoystick* JoyAxis::getParentSet()
{
    return parentSet;
}

void JoyAxis::establishPropertyUpdatedConnection()
{
    connect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
}

void JoyAxis::disconnectPropertyUpdatedConnection()
{
    disconnect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
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
    {
        relative = paxisbutton->isRelativeSpring();
    }

    return relative;
}

void JoyAxis::performCalibration(int value)
{
    InputDevice *device = parentSet->getInputDevice();
    if (value <= -30000)
    {
        // Assume axis is a trigger. Set default throttle to Positive.
        device->setCalibrationThrottle(index, PositiveThrottle);
    }
    else
    {
        // Ensure that default throttle is used when a device is reset.
        device->setCalibrationThrottle(index,
                                       static_cast<JoyAxis::ThrottleTypes>(throttle));
    }
    //else if (value >= -15000 && value <= 15000)
    //{
    //    device->setCalibrationThrottle(index, NormalThrottle);
    //}
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
    {
        emit propertyUpdated();
    }
}

void JoyAxis::setButtonsEasingDuration(double value)
{
    paxisbutton->setEasingDuration(value);
    naxisbutton->setEasingDuration(value);
}

double JoyAxis::getButtonsEasingDuration()
{
    double result = JoyButton::DEFAULTEASINGDURATION;
    if (paxisbutton->getEasingDuration() == naxisbutton->getEasingDuration())
    {
        result = paxisbutton->getEasingDuration();
    }

    return result;
}

int JoyAxis::getLastKnownThrottleValue()
{
    return lastKnownThottledValue;
}

int JoyAxis::getLastKnownRawValue()
{
    return lastKnownRawValue;
}

/**
 * @brief Determine an appropriate release value for an axis depending
 *     on the current throttle setting being used.
 * @return Release value for an axis
 */
int JoyAxis::getProperReleaseValue()
{
    // Handles NormalThrottle case
    int value = 0;

    if (throttle == NegativeHalfThrottle)
    {
        value = 0;
    }
    else if (throttle == NegativeThrottle)
    {
        value = JoyAxis::AXISMAX;
    }
    else if (throttle == PositiveThrottle)
    {
        value = JoyAxis::AXISMIN;
    }
    else if (throttle == PositiveHalfThrottle)
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
