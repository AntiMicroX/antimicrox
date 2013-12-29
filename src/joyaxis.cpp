#include <QDebug>
#include <cmath>

#include "joyaxis.h"
#include "joycontrolstick.h"
#include "event.h"

const int JoyAxis::AXISMIN = -32767;
const int JoyAxis::AXISMAX = 32767;
const int JoyAxis::AXISDEADZONE = 6000;
const int JoyAxis::AXISMAXZONE = 32000;

// Speed in pixels/second
const float JoyAxis::JOYSPEED = 20.0;


JoyAxis::JoyAxis(QObject *parent) :
    QObject(parent)
{
    originset = 0;
    stick = 0;
    naxisbutton = new JoyAxisButton(this, 0, originset);
    paxisbutton = new JoyAxisButton(this, 1, originset);

    reset();
    index = 0;
}

JoyAxis::JoyAxis(int index, int originset, QObject *parent) :
    QObject(parent)
{
    stick = 0;
    this->originset = originset;
    naxisbutton = new JoyAxisButton(this, 0, originset);
    paxisbutton = new JoyAxisButton(this, 1, originset);

    reset();
    this->index = index;
}

JoyAxis::~JoyAxis()
{
    delete paxisbutton;
    delete naxisbutton;
}

void JoyAxis::joyEvent(int value, bool ignoresets)
{
    setCurrentRawValue(value);
    //currentRawValue = value;
    bool safezone = !inDeadZone(currentRawValue);
    currentThrottledValue = calculateThrottledValue(value);

    if (this->stick)
    {
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

        stick->joyEvent(ignoresets);
    }
    else
    {
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
            label.append(naxisbutton->getSlotsSummary());
        }

        label.append(" | +");
        if (!paxisbutton->getActionName().isEmpty() && displayNames)
        {
            label.append(paxisbutton->getActionName());
        }
        else
        {
            label.append(paxisbutton->getSlotsSummary());
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
            label.append(paxisbutton->getSlotsSummary());
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
            label.append(naxisbutton->getSlotsSummary());
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
    if (throttle == NegativeThrottle)
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
    }
    else
    {
        maxZoneValue = value;
    }
}

int JoyAxis::getMaxZoneValue()
{
    return maxZoneValue;
}

void JoyAxis::setThrottle(int value)
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
    if (xml->isStartElement() && xml->name() == "axis")
    {
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "axis"))
        {
            if (xml->name() == "deadZone" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setDeadZone(tempchoice);
            }
            else if (xml->name() == "maxZone" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMaxZoneValue(tempchoice);
            }
            else if (xml->name() == "throttle" && xml->isStartElement())
            {
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

                setCurrentRawValue(currentThrottledDeadValue);
                //currentRawValue = currentThrottledDeadValue;
                currentThrottledValue = calculateThrottledValue(currentRawValue);
            }
            else if (xml->name() == JoyAxisButton::xmlName && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                if (index == 1)
                {
                    naxisbutton->readConfig(xml);
                }
                else if (index == 2)
                {
                    paxisbutton->readConfig(xml);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyAxis::writeConfig(QXmlStreamWriter *xml)
{
    if (!isDefault())
    {
        xml->writeStartElement("axis");
        xml->writeAttribute("index", QString::number(index+1));

        xml->writeTextElement("deadZone", QString::number(deadZone));
        xml->writeTextElement("maxZone", QString::number(maxZoneValue));

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

        naxisbutton->writeConfig(xml);
        paxisbutton->writeConfig(xml);

        xml->writeEndElement();
    }
}

void JoyAxis::reset()
{
    deadZone = AXISDEADZONE;
    isActive = false;

    //timer->stop();
    //interval = QTime ();
    eventActive = false;
    maxZoneValue = AXISMAXZONE;
    throttle = 0;
    //sumDist = 0.0;

    paxisbutton->reset();
    naxisbutton->reset();
    activeButton = 0;

    adjustRange();
    setCurrentRawValue(currentThrottledDeadValue);
    //currentRawValue = currentThrottledDeadValue;
    currentThrottledValue = calculateThrottledValue(currentRawValue);
    axisName.clear();
}

void JoyAxis::reset(int index)
{
    reset();
    this->index = index;
}

double JoyAxis::calculateNormalizedAxisPlacement()
{
    double difference = (abs(currentThrottledValue))/(double)(maxZoneValue);
    if (difference > 1.0)
    {
        difference = 1.0;
    }
    else if (difference < 0.0)
    {
        difference = 0.0;
    }

    return difference;
}

double JoyAxis::getAbsoluteAxisPlacement()
{
    double difference = (abs(currentRawValue))/(double)(maxZoneValue);
    if (difference > 1.0)
    {
        difference = 1.0;
    }
    else if (difference < 0.0)
    {
        difference = 0.0;
    }

    return difference;
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
    if (throttle == -1)
    {
        currentThrottledDeadValue = AXISMAX;
        //currentThrottledMin = AXISMAX;
        //currentThrottledMax = 0;
        //value = (value + AXISMIN) / 2;
    }
    else if (throttle == 0)
    {
        currentThrottledDeadValue = 0;
        //currentThrottledMin = AXISMIN;
        //currentThrottledMax = AXISMAX;
    }
    else if (throttle == 1)
    {
        currentThrottledDeadValue = AXISMIN;
        //currentThrottledMin = 0;
        //currentThrottledMax = AXISMAX;
        //value = (value + AXISMAX) / 2;
    }

    currentThrottledValue = calculateThrottledValue(currentRawValue);
}

/*int JoyAxis::getCurrentThrottledMin()
{
    return currentThrottledMin;
}

int JoyAxis::getCurrentThrottledMax()
{
    return currentThrottledMax;
}*/

int JoyAxis::getCurrentThrottledDeadValue()
{
    return currentThrottledDeadValue;
}

double JoyAxis::getDistanceFromDeadZone()
{
    double distance = 0.0;

    if (currentThrottledValue >= deadZone)
    {
        distance = (currentThrottledValue - deadZone)/(double)(maxZoneValue - deadZone);
    }
    else if (currentThrottledValue <= -deadZone)
    {
        distance = (currentThrottledValue + deadZone)/(double)(-maxZoneValue + deadZone);
    }

    if (distance > 1.0)
    {
        distance = 1.0;
    }
    else if (distance < 0.0)
    {
        distance = 0.0;
    }

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
}

bool JoyAxis::isPartControlStick()
{
    return (this->stick != 0);
}

JoyControlStick* JoyAxis::getControlStick()
{
    return this->stick;
}

void JoyAxis::removeControlStick()
{
    if (stick)
    {
        stick->releaseButtonEvents();
        this->stick = 0;
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
    value = value && (deadZone == AXISDEADZONE);
    value = value && (maxZoneValue == AXISMAXZONE);
    value = value && (throttle == 0);
    value = value && (paxisbutton->isDefault());
    value = value && (naxisbutton->isDefault());
    return value;
}

/* Use this method to keep currentRawValue in the expected range.
 * SDL has a minimum axis value of -32768 which should be ignored to
 * ensure that JoyControlStick will not encounter overflow problems
 * on a 32bit machine.
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

void JoyAxis::setButtonsSmoothing(bool enabled)
{
    paxisbutton->setSmoothing(enabled);
    naxisbutton->setSmoothing(enabled);
}

bool JoyAxis::getButtonsPresetSmoothing()
{
    bool presetSmoothing = false;

    if (paxisbutton->isSmoothingEnabled() == naxisbutton->isSmoothingEnabled())
    {
        presetSmoothing = paxisbutton->isSmoothingEnabled();
    }

    return presetSmoothing;
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
