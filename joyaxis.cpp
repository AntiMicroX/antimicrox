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
const int JoyAxis::JOYINTERVAL = 50;
// Speed in pixels/second
const float JoyAxis::JOYSPEED = 20.0;


JoyAxis::JoyAxis(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer ();
    reset();
    index = 0;
}

JoyAxis::JoyAxis(int index, QObject *parent) :
    QObject(parent)
{
    timer = new QTimer ();
    reset();
    this->index = index;
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

    if (axisMode == KeyboardAxis)
    {
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
    }
    else if (axisMode == MouseAxis)
    {
        if (safezone)
        {
            if (!isActive)
            {
                isActive = eventActive = true;
                emit active(value);
                createDeskEvent();

                connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
                sumDist = 0.0;
                timer->start(0);
                interval.start();
            }
        }
        else
        {
            isActive = eventActive = false;
            interval.restart();
            timer->stop();
            sumDist = 0.0;
            disconnect(timer, SIGNAL(timeout()), 0, 0);
            emit released(value);
        }
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
    QString label = QString("Axis ").append(QString::number(getRealJoyIndex()))
            .append(": ");
    if (axisMode == KeyboardAxis)
    {
        label = label.append("[KEYBOARD]");
    }
    else if (axisMode == MouseAxis)
    {
        label = label.append("[MOUSE]");
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

void JoyAxis::setPKey(int code)
{
    pkeycode = code;
}

int JoyAxis::getPKey()
{
    return pkeycode;
}

void JoyAxis::setNKey(int code)
{
    nkeycode = code;
}

int JoyAxis::getNKey()
{
    return nkeycode;
}

void JoyAxis::createDeskEvent()
{
    if (axisMode == KeyboardAxis)
    {
        JoyInputDevice currentMode = JoyKeyboard;
        int temppkey = pkeycode;
        int tempnkey = nkeycode;

        if (currentValue > deadZone)
        {
            if (pkeycode > mouseOffset)
            {
                temppkey -= mouseOffset;
                currentMode = JoyMouse;
            }
            sendevent(temppkey, eventActive, currentMode);
            lastkey = pkeycode;
        }
        else if (currentValue < -deadZone)
        {
            if (nkeycode > mouseOffset)
            {
                tempnkey -= mouseOffset;
                currentMode = JoyMouse;
            }
            sendevent(tempnkey, eventActive, currentMode);
            lastkey = nkeycode;
        }
        else if (lastkey > 0)
        {
            int templast = lastkey;
            if (lastkey > mouseOffset)
            {
                templast -= mouseOffset;
                currentMode = JoyMouse;
            }
            sendevent(templast, eventActive, currentMode);
            lastkey = 0;
        }
    }
    else if (axisMode == MouseAxis)
    {
        if (interval.elapsed() >= (1000.0/(mouseSpeed*JOYSPEED)))
        {
            float difference = (abs(currentValue) - deadZone)/(float)(maxZoneValue - deadZone);
            sumDist += difference;

            int distance = (int)floor (sumDist + 0.5);
            if (currentValue < 0)
            {
                distance = -distance;
            }

            int mouse1 = 0;
            int mouse2 = 0;
            if (mousemode == MouseHorizontal)
            {
                mouse1 = distance;
            }
            else if (mousemode == MouseInvHorizontal)
            {
                mouse1 = -distance;
            }
            else if (mousemode == MouseVertical)
            {
                mouse2 = distance;
            }
            else if (mousemode == MouseInvVertical)
            {
                mouse2 = -distance;
            }

            if (sumDist >= 1.0)
            {
                sendevent(mouse1, mouse2);
                sumDist = 0.0;
            }

            interval.restart();
        }
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


void JoyAxis::setMouseMode(int mode)
{
    JoyAxisMouseMode temp = (JoyAxisMouseMode) mode;
    switch (mode) {
        case MouseHorizontal:
        case MouseInvHorizontal:
        case MouseVertical:
        case MouseInvVertical:
        {
            mousemode = temp;
            break;
        }

        default:
        {
            break;
        }
    }
}

int JoyAxis::getMouseMode()
{
    int mode = (int)mousemode;
    return mode;
}

void JoyAxis::setMouseSpeed(int speed)
{
    if (speed >= 1 && speed <= 50)
    {
        mouseSpeed = speed;
    }
}

int JoyAxis::getMouseSpeed()
{
    return mouseSpeed;
}

void JoyAxis::setAxisMode(int mode)
{
    JoyAxisMode temp = (JoyAxisMode) mode;
    switch (mode)
    {
        case KeyboardAxis:
        case MouseAxis:
        {
            axisMode = temp;
            break;
        }

        default:
        {
            break;
        }
    }
}

int JoyAxis::getAxisMode()
{
    int temp = (int) axisMode;
    return temp;
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

            else if (xml->name() == "pkeycode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setPKey(tempchoice);
            }
            else if (xml->name() == "nkeycode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setNKey(tempchoice);
            }
            else if (xml->name() == "axismode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setAxisMode(tempchoice);
            }
            else if (xml->name() == "mousemode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseMode(tempchoice);
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
            else if (xml->name() == "mousespeed" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseSpeed(tempchoice);
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
    xml->writeTextElement("nkeycode", QString::number(nkeycode));
    xml->writeTextElement("pkeycode", QString::number(pkeycode));
    xml->writeTextElement("axismode", QString::number(axisMode));
    xml->writeTextElement("mousemode", QString::number(mousemode));
    xml->writeTextElement("mousespeed", QString::number(mouseSpeed));

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

    xml->writeEndElement();
}

void JoyAxis::timerEvent()
{
    if (axisMode == KeyboardAxis)
    {
        eventActive = !eventActive;
        createDeskEvent();
    }
    else
    {
        eventActive = true;
        createDeskEvent();
    }
}

void JoyAxis::reset()
{
    deadZone = 5000;
    isActive = false;
    mouseSpeed = 20;
    axisMode = KeyboardAxis;
    timer->stop();
    interval = QTime ();
    eventActive = false;
    currentValue = 0;
    maxZoneValue = 30000;
    throttle = 0;
    sumDist = 0.0;
    mouseOffset = 400;
    lastkey = 0;
    pkeycode = 0;
    nkeycode = 0;
}

void JoyAxis::reset(int index)
{
    reset();
    this->index = index;
}

JoyAxis::~JoyAxis()
{
    if (timer)
    {
        timer->stop();
        delete timer;
        timer = 0;
    }
}
