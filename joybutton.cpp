#include <QDebug>
#include <QThread>

#include "joybutton.h"
#include "event.h"

const QString JoyButton::xmlName = "button";

JoyButton::JoyButton(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer (this);
    this->reset();
    index = 0;
}

JoyButton::JoyButton(int index, QObject *parent) :
    QObject(parent)
{
    timer = new QTimer (this);
    this->reset();
    this->index = index;
}

void JoyButton::joyEvent(bool pressed)
{
    if (toggle && pressed && (pressed != isDown))
    {
        isButtonPressed = !isButtonPressed;
        isDown = true;
        emit clicked(index);
        createDeskEvent();
    }
    else if (toggle && !pressed && isDown)
    {
        isDown = false;
        emit released(index);
    }

    else if (!toggle && (pressed != isButtonPressed))
    {
        if (pressed)
        {
            emit clicked(index);
        }
        else
        {
            emit released(index);
        }

        isButtonPressed = pressed;

        if (isButtonPressed && useTurbo)
        {
            turboEvent();
            connect(timer, SIGNAL(timeout()), this, SLOT(turboEvent()));
        }
        else if (!isButtonPressed && useTurbo)
        {
            timer->stop();
            disconnect(timer, SIGNAL(timeout()), 0, 0);
            if (isKeyPressed)
            {
                turboEvent();
            }
        }

        if (!useTurbo)
        {
            createDeskEvent();
        }
    }
}

int JoyButton::getJoyNumber()
{
    return index;
}

int JoyButton::getRealJoyNumber()
{
    return index + 1;
}

void JoyButton::setJoyNumber(int index)
{
    this->index = index;
}

void JoyButton::setKey(int keycode)
{
    this->keycode = keycode;
    emit keyChanged(keycode);
}

void JoyButton::setMouse(int mouse)
{
    if (mouse > 0 && mouse <= 8)
    {
        this->mousecode = mouse;
        emit mouseChanged(mouse);
    }
}

int JoyButton::getMouse()
{
    return mousecode;
}

void JoyButton::setToggle(bool toggle)
{
    this->toggle = toggle;
}

void JoyButton::setTurboInterval(int interval)
{
    this->turboInterval = interval;
}

void JoyButton::reset()
{
    isKeyPressed = isButtonPressed = false;
    toggle = false;
    turboInterval = 0;
    keycode = 0;
    mousecode = 0;
    isDown = false;
    useMouse = false;
    useTurbo = false;

    if (timer)
    {
        timer->stop();
    }
}

void JoyButton::reset(int index)
{
    JoyButton::reset();
    this->index = index;
}

int JoyButton::getKey()
{
    return keycode;
}

bool JoyButton::getToggleState()
{
    return toggle;
}

int JoyButton::getTurboInterval()
{
    return turboInterval;
}

void JoyButton::turboEvent()
{
    if (!isKeyPressed)
    {
        sendevent(keycode, true);
        isKeyPressed = true;
        timer->start(100);
    }
    else
    {
        sendevent(keycode, false);
        isKeyPressed = false;
        timer->start(turboInterval - 100);
    }
}

JoyButton::~JoyButton()
{
    if (timer)
    {
        timer->stop();
    }
}

void JoyButton::setUseMouse(bool useMouse)
{
    this->useMouse = useMouse;
}

bool JoyButton::isUsingMouse()
{
    return useMouse;
}

void JoyButton::createDeskEvent()
{
    if (!useMouse && keycode > 0)
    {
        sendevent(keycode, isButtonPressed);
    }
    else if (useMouse && mousecode > 0)
    {
        sendevent(mousecode, isButtonPressed, JoyMouse);
    }
}

void JoyButton::setUseTurbo(bool useTurbo)
{
    this->useTurbo = useTurbo;
}

bool JoyButton::isUsingTurbo()
{
    return useTurbo;
}

QString JoyButton::getXmlName()
{
    return this->xmlName;
}

void JoyButton::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            if (xml->name() == "toggle" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setToggle(true);
                }
            }
            else if (xml->name() == "turbointerval" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setTurboInterval(tempchoice);
            }
            else if (xml->name() == "useturbo" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setUseTurbo(true);
                }
            }
            else if (xml->name() == "keycode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setKey(tempchoice);
            }
            else if (xml->name() == "usemouse" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setUseMouse(true);
                }
            }
            else if (xml->name() == "mousecode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouse(tempchoice);
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }

}

void JoyButton::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number(getRealJoyNumber()));

    xml->writeTextElement("keycode", QString::number(keycode));
    xml->writeTextElement("usemouse", useMouse ? "true" : "false");
    xml->writeTextElement("mousecode", QString::number(mousecode));
    xml->writeTextElement("toggle", toggle ? "true" : "false");
    xml->writeTextElement("turbointerval", QString::number(turboInterval));
    xml->writeTextElement("useturbo", useTurbo ? "true" : "false");

    xml->writeEndElement();
}

QString JoyButton::getName()
{
    QString newlabel = getPartialName();
    if (keycode > 0)
    {
        newlabel = newlabel.append(": ").append(keycodeToKey(keycode).toUpper());
    }
    else if (mousecode > 0)
    {
        newlabel = newlabel.append(": Mouse ").append(QString::number(mousecode));
    }
    else
    {
        newlabel = newlabel.append(": [NO KEY]");
    }

    return newlabel;
}

QString JoyButton::getPartialName()
{
    return QString("Button ").append(QString::number(getRealJoyNumber()));
}
