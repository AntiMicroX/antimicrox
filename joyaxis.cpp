#include <QDebug>
#include <math.h>

#include "joyaxis.h"
#include "event.h"

const int JoyAxis::AXISMIN = -32768;
const int JoyAxis::AXISMAX = 32767;
const int JoyAxis::AXISDEADZONE = 5000;
const int JoyAxis::AXISMAXZONE = 30000;

// Set event interval to allow one event every 50 ms.
// Should create around 20 events per second.
// mouseSpeed * 20 will give the number of pixels per second
// the mouse cursor will move on screen

// Speed in pixels/second
const float JoyAxis::JOYSPEED = 20.0;


JoyAxis::JoyAxis(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer ();
    naxisbutton = new JoyAxisButton(this, 0);
    paxisbutton = new JoyAxisButton(this, 1);

    reset();
    index = 0;
}

JoyAxis::JoyAxis(int index, QObject *parent) :
    QObject(parent)
{
    timer = new QTimer ();
    naxisbutton = new JoyAxisButton(this, 0);
    paxisbutton = new JoyAxisButton(this, 1);

    reset();

    this->index = index;
}

JoyAxis::~JoyAxis()
{
    delete timer;
    delete paxisbutton;
    delete naxisbutton;
}

void JoyAxis::joyEvent(int value)
{
    int temp = value;
    if (throttle == -1)
    {
        value = (value + AXISMIN) / 2;
    }
    else if (throttle == 1)
    {
        value = (value + AXISMAX) / 2;
    }

    bool safezone = !inDeadZone(temp);
    currentValue = value;

    if (safezone && !isActive)
    {
        isActive = eventActive = true;
        emit active(value);
        createDeskEvent();
    }
    else if (!safezone && isActive)
    {
        isActive = eventActive = false;
        emit released(value);

        createDeskEvent();
        lastkey = 0;
    }

    emit moved(temp);
}

bool JoyAxis::inDeadZone(int value)
{
    bool result = false;
    if (throttle == -1)
    {
        value = (value + AXISMIN) / 2;
    }
    else if (throttle == 1)
    {
        value = (value + AXISMAX) / 2;
    }

    if (abs(value) <= deadZone)
    {
        result = true;
    }

    return result;
}

QString JoyAxis::getName()
{
    QString label = QString("Axis ").append(QString::number(getRealJoyIndex()));
    label.append(": ");

    if (throttle == 0)
    {
        label.append("-").append(naxisbutton->getSlotsSummary());
        label.append(" | +").append(paxisbutton->getSlotsSummary());
    }
    else if (throttle == 1)
    {
        label.append("+").append(paxisbutton->getSlotsSummary());
    }
    else if (throttle == -1)
    {
        label.append("-").append(naxisbutton->getSlotsSummary());
    }

    return label;
}

int JoyAxis::getRealJoyIndex()
{
    return index + 1;
}

int JoyAxis::getCurrentValue()
{
    return currentValue;
}

void JoyAxis::setIndex(int index)
{
    this->index = index;
}

int JoyAxis::getIndex()
{
    return index;
}


void JoyAxis::createDeskEvent()
{
    if (currentValue > deadZone)
    {
        paxisbutton->joyEvent(eventActive);
        activeButton = paxisbutton;
    }
    else if (currentValue < -deadZone)
    {
        naxisbutton->joyEvent(eventActive);
        activeButton = naxisbutton;
    }
    else if (activeButton)
    {
        activeButton->joyEvent(eventActive);
        activeButton = 0;
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
    if (value >= -1 && value <= 1)
    {
        throttle = value;
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
        reset();

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
                if (temptext == "negative")
                {
                    this->setThrottle(-1);
                }
                else if (temptext == "normal")
                {
                    this->setThrottle(0);
                }
                else if (temptext == "positive")
                {
                    this->setThrottle(1);
                }
            }
            else if (xml->name() == "axisbutton" && xml->isStartElement())
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
    xml->writeStartElement("axis");
    xml->writeAttribute("index", QString::number(index+1));

    xml->writeTextElement("deadZone", QString::number(deadZone));
    xml->writeTextElement("maxZone", QString::number(maxZoneValue));

    xml->writeStartElement("throttle");
    if (throttle == -1)
    {
        xml->writeCharacters("negative");
    }
    else if (throttle == 0)
    {
        xml->writeCharacters("normal");
    }
    else if (throttle == 1)
    {
        xml->writeCharacters("positive");
    }
    xml->writeEndElement();

    naxisbutton->writeConfig(xml);
    paxisbutton->writeConfig(xml);

    xml->writeEndElement();
}

void JoyAxis::reset()
{
    deadZone = 5000;
    isActive = false;

    timer->stop();
    interval = QTime ();
    eventActive = false;
    currentValue = 0;
    maxZoneValue = 30000;
    throttle = 0;
    sumDist = 0.0;
    mouseOffset = 400;
    lastkey = 0;

    paxisbutton->reset();
    naxisbutton->reset();
    activeButton = 0;
}

void JoyAxis::reset(int index)
{
    reset();
    this->index = index;
}

double JoyAxis::calculateNormalizedAxisPlacement()
{
    double difference = (abs(currentValue) - deadZone)/(double)(maxZoneValue - deadZone);
    if (difference > 1.0)
    {
        difference = 1.0;
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
