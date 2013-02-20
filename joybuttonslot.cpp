#include "joybuttonslot.h"

const int JoyButtonSlot::JOYSPEED = 20;
const QString JoyButtonSlot::xmlName = "slot";


JoyButtonSlot::JoyButtonSlot(QObject *parent) :
    QObject(parent)
{
    deviceCode = 0;
    mode = JoyKeyboard;
    distance = 0.0;
    mouseInterval = new QTime();
}

JoyButtonSlot::JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent) :
    QObject(parent)
{
    if (code > 0)
    {
        deviceCode = code;
    }
    else
    {
        deviceCode = 0;
    }

    this->mode = mode;
    distance = 0.0;
    mouseInterval = new QTime();
}

JoyButtonSlot::~JoyButtonSlot()
{
    delete mouseInterval;
}

void JoyButtonSlot::setSlotCode(int code)
{
    deviceCode = code;
}

int JoyButtonSlot::getSlotCode()
{
    return deviceCode;
}

void JoyButtonSlot::setSlotMode(JoySlotInputAction selectedMode)
{
    mode = selectedMode;
}

JoyButtonSlot::JoySlotInputAction JoyButtonSlot::getSlotMode()
{
    return mode;
}

QString JoyButtonSlot::movementString()
{
    QString newlabel;

    if (mode == JoyMouseMovement)
    {
        newlabel.append("Mouse ");
        if (deviceCode == JoyButtonSlot::MouseUp)
        {
            newlabel.append("Up");
        }
        else if (deviceCode == JoyButtonSlot::MouseDown)
        {
            newlabel.append("Down");
        }
        else if (deviceCode == JoyButtonSlot::MouseLeft)
        {
            newlabel.append("Left");
        }
        else if (deviceCode == JoyButtonSlot::MouseRight)
        {
            newlabel.append("Right");
        }
    }

    return newlabel;
}

void JoyButtonSlot::setDistance(double distance)
{
    this->distance = distance;
}

double JoyButtonSlot::getDistance()
{
    return distance;
}

QTime* JoyButtonSlot::getMouseInterval()
{
    return mouseInterval;
}

void JoyButtonSlot::restartMouseInterval()
{
    mouseInterval->restart();
}

void JoyButtonSlot::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "slot")
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "slot"))
        {
            if (xml->name() == "code" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setSlotCode(tempchoice);
            }
            else if (xml->name() == "mode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (temptext == "keyboard")
                {
                    this->setSlotMode(JoyKeyboard);
                }
                else if (temptext == "mousebutton")
                {
                    this->setSlotMode(JoyMouseButton);
                }
                else if (temptext == "mousemovement")
                {
                    this->setSlotMode(JoyMouseMovement);
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

void JoyButtonSlot::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());

    xml->writeTextElement("code", QString::number(deviceCode));
    xml->writeStartElement("mode");
    if (mode == JoyKeyboard)
    {
        xml->writeCharacters("keyboard");
    }
    else if (mode == JoyMouseButton)
    {
        xml->writeCharacters("mousebutton");
    }
    else if (mode == JoyMouseMovement)
    {
        xml->writeCharacters("mousemovement");
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

QString JoyButtonSlot::getXmlName()
{
    return this->xmlName;
}
