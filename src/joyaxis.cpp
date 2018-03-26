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
#include "joycontrolstick.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "event.h"

#include <cmath>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    stick = nullptr;
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    reset();
}

void JoyAxis::queuePendingEvent(int value, bool ignoresets, bool updateLastValues)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    pendingEvent = false;
    pendingValue = 0;
    pendingIgnoreSets = false;
    //pendingUpdateLastValues = true;

    if (this->stick != nullptr)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return pendingEvent;
}

void JoyAxis::clearPendingEvent()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    pendingEvent = false;
    pendingValue = false;
    pendingIgnoreSets = false;
}

void JoyAxis::stickPassEvent(int value, bool ignoresets, bool updateLastValues)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (this->stick != nullptr)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((this->stick != nullptr) && !pendingEvent)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = false;
    int temp = calculateThrottledValue(value);

    if (abs(temp) <= deadZone)
    {
        qDebug() << "Value of throttle is in (less than) dead zone: " << abs(temp) << " <= " << deadZone;
        result = true;

    } else {

       qDebug() << "Value of throttle is not in (greater than) dead zone: " << abs(temp) << " > " << deadZone;
    }

    return result;
}

QString JoyAxis::getName(bool forceFullFormat, bool displayNames)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return index + 1;
}

int JoyAxis::getCurrentThrottledValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return currentThrottledValue;
}

int JoyAxis::calculateThrottledValue(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    qDebug() << "Throtted value at start of function is: " << value;
    int temp = value;

    if (throttle == static_cast<int>(NegativeHalfThrottle))
    {
        qDebug() << "It's a negative half throttle";
        value = (value <= 0) ? value : -value;
        temp = value;
    }
    else if (throttle == static_cast<int>(NegativeThrottle))
    {
        qDebug() << "It's a negative throttle";

        temp = (value + AXISMIN) / 2;
    }
    else if (throttle == static_cast<int>(PositiveThrottle))
    {
        qDebug() << "It's a positive throttle";

        temp = (value + AXISMAX) / 2;
    }
    else if (throttle == static_cast<int>(PositiveHalfThrottle))
    {
        qDebug() << "It's a positive half throttle";

        value = (value >= 0) ? value : -value;
        temp = value;
    }

    qDebug() << "Calculated value of throttle is: " << temp;
    return temp;
}

void JoyAxis::setIndex(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->index = index;
}

int JoyAxis::getIndex()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return index;
}


void JoyAxis::createDeskEvent(bool ignoresets)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxisButton *eventbutton = nullptr;
    if (currentThrottledValue > deadZone)
    {
        eventbutton = paxisbutton;
    }
    else if (currentThrottledValue < -deadZone)
    {
        eventbutton = naxisbutton;
    }

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deadZone = abs(value);
    emit propertyUpdated();
}

int JoyAxis::getDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return deadZone;
}

void JoyAxis::setMaxZoneValue(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return maxZoneValue;
}

/**
 * @brief Set throttle value for axis.
 * @param Current value for axis.
 */
void JoyAxis::setThrottle(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    qDebug() << "Value of throttle for axis in setThrottle is: " << value;
    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) && (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle)))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((value >= static_cast<int>(JoyAxis::NegativeHalfThrottle)) && (value <= static_cast<int>(JoyAxis::PositiveHalfThrottle)))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return throttle;
}

void JoyAxis::readConfig(QXmlStreamReader *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (xml->isStartElement() && (xml->name() == getXmlName()))
    {
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != getXmlName())))
        {
            bool found = false;
            found = readMainConfig(xml);
            if (!found && (xml->name() == naxisbutton->getXmlName()) && xml->isStartElement())
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

        if (throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle))
        {
            xml->writeCharacters("negativehalf");
        }
        else if (throttle == static_cast<int>(JoyAxis::NegativeThrottle))
        {
            xml->writeCharacters("negative");
        }
        else if (throttle == static_cast<int>(JoyAxis::NormalThrottle))
        {
            xml->writeCharacters("normal");
        }
        else if (throttle == static_cast<int>(JoyAxis::PositiveThrottle))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool found = false;

    if ((xml->name() == "deadZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        qDebug() << "From xml config dead zone is: " << tempchoice;
        this->setDeadZone(tempchoice);
    }
    else if ((xml->name() == "maxZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        qDebug() << "From xml config max zone is: " << tempchoice;
        this->setMaxZoneValue(tempchoice);
    }
    else if ((xml->name() == "throttle") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        qDebug() << "From xml config throttle name is: " << temptext;

        if (temptext == "negativehalf")
        {
            this->setThrottle(static_cast<int>(JoyAxis::NegativeHalfThrottle));
        }
        else if (temptext == "negative")
        {
            this->setThrottle(static_cast<int>(JoyAxis::NegativeThrottle));
        }
        else if (temptext == "normal")
        {
            this->setThrottle(static_cast<int>(JoyAxis::NormalThrottle));
        }
        else if (temptext == "positive")
        {
            this->setThrottle(static_cast<int>(JoyAxis::PositiveThrottle));
        }
        else if (temptext == "positivehalf")
        {
            this->setThrottle(static_cast<int>(JoyAxis::PositiveHalfThrottle));
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    deadZone = getDefaultDeadZone();
    isActive = false;

    eventActive = false;
    maxZoneValue = getDefaultMaxZone();
    throttle = getDefaultThrottle();

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
    //pendingUpdateLastValues = true;
}

void JoyAxis::reset(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    reset();
    this->index = index;
}

JoyAxisButton* JoyAxis::getPAxisButton()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return paxisbutton;
}

JoyAxisButton* JoyAxis::getNAxisButton()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return naxisbutton;
}

int JoyAxis::getCurrentRawValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return currentRawValue;
}

void JoyAxis::adjustRange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (throttle == static_cast<int>(JoyAxis::NegativeThrottle))
    {
        currentThrottledDeadValue = AXISMAX;
    }
    else if ((throttle == static_cast<int>(JoyAxis::NormalThrottle)) ||
             (throttle == static_cast<int>(JoyAxis::PositiveHalfThrottle)) ||
             (throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        currentThrottledDeadValue = 0;
    }
    else if (throttle == static_cast<int>(JoyAxis::PositiveThrottle))
    {
        currentThrottledDeadValue = AXISMIN;
    }

    currentThrottledValue = calculateThrottledValue(currentRawValue);
}

int JoyAxis::getCurrentThrottledDeadValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return currentThrottledDeadValue;
}

double JoyAxis::getDistanceFromDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return getDistanceFromDeadZone(currentThrottledValue);
}

double JoyAxis::getDistanceFromDeadZone(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double distance = 0.0;
    int currentValue = value;

    if (currentValue >= deadZone)
    {
        distance = static_cast<double>((currentValue - deadZone)/(maxZoneValue - deadZone));
    }
    else if (currentValue <= -deadZone)
    {
        distance = static_cast<double>((currentValue + deadZone)/( (-maxZoneValue) + deadZone));
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double distance = 0.0;
    int currentValue = value;

    distance = static_cast<double>(currentValue / maxZoneValue);
    distance = qBound(-1.0, distance, 1.0);

    return distance;
}

void JoyAxis::propogateThrottleChange()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    emit throttleChangePropogated(this->index);
}

int JoyAxis::getCurrentlyAssignedSet()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return originset;
}

void JoyAxis::setControlStick(JoyControlStick *stick)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    removeVDPads();
    removeControlStick();
    this->stick = stick;
    emit propertyUpdated();
}

bool JoyAxis::isPartControlStick()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return (this->stick != nullptr);
}

JoyControlStick* JoyAxis::getControlStick()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->stick;
}

void JoyAxis::removeControlStick(bool performRelease)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (stick != nullptr)
    {
        if (performRelease)
        {
            stick->releaseButtonEvents();
        }

        this->stick = nullptr;
        emit propertyUpdated();
    }
}


bool JoyAxis::hasControlOfButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool value = true;
    if (paxisbutton->isPartVDPad() || naxisbutton->isPartVDPad())
    {
        value = false;
    }

    return value;
}

void JoyAxis::removeVDPads()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((value >= JoyAxis::AXISMIN) && (value <= JoyAxis::AXISMAX))
    {
        qDebug() << "Raw value is less than " << JoyAxis::AXISMAX << " and greather than " << JoyAxis::AXISMIN;
        currentRawValue = value;
    }
    else if (value > JoyAxis::AXISMAX)
    {
        qDebug() << "Raw value is greather than " << JoyAxis::AXISMAX;

        currentRawValue = JoyAxis::AXISMAX;
    }
    else if (value < JoyAxis::AXISMIN)
    {
        qDebug() << "Raw value is less than " << JoyAxis::AXISMIN;

        currentRawValue = JoyAxis::AXISMIN;
    }

    qDebug() << "Raw value for axis is: " << currentRawValue;
}

void JoyAxis::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setMouseMode(mode);
    naxisbutton->setMouseMode(mode);
}

bool JoyAxis::hasSameButtonsMouseMode()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = true;
    if (paxisbutton->getMouseMode() != naxisbutton->getMouseMode())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyAxis::getButtonsPresetMouseMode()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;
    if (paxisbutton->getMouseMode() == naxisbutton->getMouseMode())
    {
        resultMode = paxisbutton->getMouseMode();
    }

    return resultMode;
}

void JoyAxis::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setMouseCurve(mouseCurve);
    naxisbutton->setMouseCurve(mouseCurve);
}

bool JoyAxis::hasSameButtonsMouseCurve()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool result = true;
    if (paxisbutton->getMouseCurve() != naxisbutton->getMouseCurve())
    {
        result = false;
    }

    return result;
}

JoyButton::JoyMouseCurve JoyAxis::getButtonsPresetMouseCurve()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;
    if (paxisbutton->getMouseCurve() == naxisbutton->getMouseCurve())
    {
        resultCurve = paxisbutton->getMouseCurve();
    }

    return resultCurve;
}

void JoyAxis::setButtonsSpringWidth(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setSpringWidth(value);
    naxisbutton->setSpringWidth(value);
}

void JoyAxis::setButtonsSpringHeight(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setSpringHeight(value);
    naxisbutton->setSpringHeight(value);
}

int JoyAxis::getButtonsPresetSpringWidth()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int presetSpringWidth = 0;

    if (paxisbutton->getSpringWidth() == naxisbutton->getSpringWidth())
    {
        presetSpringWidth = paxisbutton->getSpringWidth();
    }

    return presetSpringWidth;
}

int JoyAxis::getButtonsPresetSpringHeight()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int presetSpringHeight = 0;

    if (paxisbutton->getSpringHeight() == naxisbutton->getSpringHeight())
    {
        presetSpringHeight = paxisbutton->getSpringHeight();
    }

    return presetSpringHeight;
}

void JoyAxis::setButtonsSensitivity(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setSensitivity(value);
    naxisbutton->setSensitivity(value);
}

double JoyAxis::getButtonsPresetSensitivity()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double presetSensitivity = 1.0;

    if (paxisbutton->getSensitivity() == naxisbutton->getSensitivity())
    {
        presetSensitivity = paxisbutton->getSensitivity();
    }

    return presetSensitivity;
}

JoyAxisButton* JoyAxis::getAxisButtonByValue(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyAxisButton *eventbutton = nullptr;
    int throttledValue = calculateThrottledValue(value);

    qDebug() << "throttledValue in getAxisButtonByValue is: " << throttledValue;
    if (throttledValue > deadZone)
    {
        qDebug() << "throtted value is positive";
        eventbutton = paxisbutton;
    }
    else if (throttledValue < (-deadZone))
    {
        qDebug() << "throtted value is negative";
        eventbutton = naxisbutton;
    }

    return eventbutton;
}

void JoyAxis::setAxisName(QString tempName)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if ((tempName.length() <= 20) && (tempName != axisName))
    {
        axisName = tempName;
        emit axisNameChanged();
        emit propertyUpdated();
    }
}

QString JoyAxis::getAxisName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return axisName;
}

void JoyAxis::setButtonsWheelSpeedX(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setWheelSpeedX(value);
    naxisbutton->setWheelSpeedX(value);
}

void JoyAxis::setButtonsWheelSpeedY(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setWheelSpeedY(value);
    naxisbutton->setWheelSpeedY(value);
}

void JoyAxis::setDefaultAxisName(QString tempname)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    defaultAxisName = tempname;
}

QString JoyAxis::getDefaultAxisName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return defaultAxisName;
}

QString JoyAxis::getPartialName(bool forceFullFormat, bool displayNames)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->xmlName;
}

int JoyAxis::getDefaultDeadZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->AXISDEADZONE;
}

int JoyAxis::getDefaultMaxZone()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->AXISMAXZONE;
}

JoyAxis::ThrottleTypes JoyAxis::getDefaultThrottle()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return this->DEFAULTTHROTTLE;
}

SetJoystick* JoyAxis::getParentSet()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return parentSet;
}

void JoyAxis::establishPropertyUpdatedConnection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
}

void JoyAxis::disconnectPropertyUpdatedConnection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
}

void JoyAxis::setButtonsSpringRelativeStatus(bool value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setSpringRelativeStatus(value);
    naxisbutton->setSpringRelativeStatus(value);
}

bool JoyAxis::isRelativeSpring()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool relative = false;

    if (paxisbutton->isRelativeSpring() == naxisbutton->isRelativeSpring())
    {
        relative = paxisbutton->isRelativeSpring();
    }

    return relative;
}

void JoyAxis::performCalibration(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    //else if ((value >= -15000) && (value <= 15000))
    //{
    //    device->setCalibrationThrottle(index, static_cast<int>(NormalThrottle));
    //}
}

void JoyAxis::copyAssignments(JoyAxis *destAxis)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setEasingDuration(value);
    naxisbutton->setEasingDuration(value);
}

double JoyAxis::getButtonsEasingDuration()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double result = JoyButton::DEFAULTEASINGDURATION;
    if (paxisbutton->getEasingDuration() == naxisbutton->getEasingDuration())
    {
        result = paxisbutton->getEasingDuration();
    }

    return result;
}

int JoyAxis::getLastKnownThrottleValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return lastKnownThottledValue;
}

int JoyAxis::getLastKnownRawValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return lastKnownRawValue;
}

/**
 * @brief Determine an appropriate release value for an axis depending
 *     on the current throttle setting being used.
 * @return Release value for an axis
 */
int JoyAxis::getProperReleaseValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    // Handles NormalThrottle case
    int value = 0;

    if (throttle == static_cast<int>(NegativeHalfThrottle))
    {
        value = 0;
    }
    else if (throttle == static_cast<int>(NegativeThrottle))
    {
        value = JoyAxis::AXISMAX;
    }
    else if (throttle == static_cast<int>(PositiveThrottle))
    {
        value = JoyAxis::AXISMIN;
    }
    else if (throttle == static_cast<int>(PositiveHalfThrottle))
    {
        value = 0;
    }

    return value;
}

void JoyAxis::setExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    paxisbutton->setExtraAccelerationCurve(curve);
    naxisbutton->setExtraAccelerationCurve(curve);
}

JoyButton::JoyExtraAccelerationCurve JoyAxis::getExtraAccelerationCurve()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::JoyExtraAccelerationCurve result = JoyButton::LinearAccelCurve;
    if (paxisbutton->getExtraAccelerationCurve() == naxisbutton->getExtraAccelerationCurve())
    {
        result = paxisbutton->getExtraAccelerationCurve();
    }

    return result;
}

void JoyAxis::copyRawValues(JoyAxis *srcAxis)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->lastKnownRawValue = srcAxis->lastKnownRawValue;
    this->currentRawValue = srcAxis->currentRawValue;
}

void JoyAxis::copyThrottledValues(JoyAxis *srcAxis)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->lastKnownThottledValue = srcAxis->lastKnownThottledValue;
    this->currentThrottledValue = srcAxis->currentThrottledValue;
}

void JoyAxis::eventReset()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    naxisbutton->eventReset();
    paxisbutton->eventReset();
}
