#include <QDebug>

#include "joybuttonslot.h"

#include "antkeymapper.h"
#include "event.h"

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
    deviceCode = 0;

    if (code > 0)
    {
        deviceCode = code;
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
    if (code >= 0)
    {
        deviceCode = code;
    }
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
        newlabel.append(tr("Mouse")).append(" ");
        if (deviceCode == JoyButtonSlot::MouseUp)
        {
            newlabel.append(tr("Up"));
        }
        else if (deviceCode == JoyButtonSlot::MouseDown)
        {
            newlabel.append(tr("Down"));
        }
        else if (deviceCode == JoyButtonSlot::MouseLeft)
        {
            newlabel.append(tr("Left"));
        }
        else if (deviceCode == JoyButtonSlot::MouseRight)
        {
            newlabel.append(tr("Right"));
        }
    }

    return newlabel;
}

void JoyButtonSlot::setDistance(double distance)
{
    this->distance = distance;
}

double JoyButtonSlot::getMouseDistance()
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
                bool ok = false;
                unsigned int tempchoice = temptext.toInt(&ok, 0);
                if (ok)
                {
                    this->setSlotCode(tempchoice);
                }
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
                else if (temptext == "pause")
                {
                    this->setSlotMode(JoyPause);
                }
                else if (temptext == "hold")
                {
                    this->setSlotMode(JoyHold);
                }
                else if (temptext == "cycle")
                {
                    this->setSlotMode(JoyCycle);
                }
                else if (temptext == "distance")
                {
                    this->setSlotMode(JoyDistance);
                }
                else if (temptext == "release")
                {
                    this->setSlotMode(JoyRelease);
                }
                else if (temptext == "mousespeedmod")
                {
                    this->setSlotMode(JoyMouseSpeedMod);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
        if (this->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
/*#ifdef Q_OS_WIN
            unsigned int virtualkey = QtWinKeyMapper::returnVirtualKey(this->getSlotCode());
#else
            //unsigned int virtualkey = X11KeyCodeToX11KeySym(this->getSlotCode());
            unsigned int virtualkey = QtX11KeyMapper::returnVirtualKey(this->getSlotCode());
#endif*/
            unsigned int virtualkey = AntKeyMapper::returnVirtualKey(this->getSlotCode());
            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                this->setSlotCode(virtualkey);
            }
            else if ((unsigned int)this->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                unsigned int temp = this->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                this->setSlotCode(temp);
            }
        }
    }

}

void JoyButtonSlot::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());

    if (mode == JoyKeyboard)
    {
        unsigned int qtkey = AntKeyMapper::returnQtKey(deviceCode);
        if (qtkey > 0)
        {
            // Found a valid abstract keysym.
            //qDebug() << "ANT KEY: " << QString::number(qtkey, 16);
            xml->writeTextElement("code", QString("0x%1").arg(qtkey, 0, 16));
        }
        else if (deviceCode > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            unsigned int tempkey = deviceCode | QtKeyMapperBase::nativeKeyPrefix;
            //qDebug() << "ANT KEY: " << QString::number(tempkey, 16);
            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }

    }
    else
    {
        xml->writeTextElement("code", QString::number(deviceCode));
    }

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
    else if (mode == JoyPause)
    {
        xml->writeCharacters("pause");
    }
    else if (mode == JoyHold)
    {
        xml->writeCharacters("hold");
    }
    else if (mode == JoyCycle)
    {
        xml->writeCharacters("cycle");
    }
    else if (mode == JoyDistance)
    {
        xml->writeCharacters("distance");
    }
    else if (mode == JoyRelease)
    {
        xml->writeCharacters("release");
    }
    else if (mode == JoyMouseSpeedMod)
    {
        xml->writeCharacters("mousespeedmod");
    }

    xml->writeEndElement();

    xml->writeEndElement();
}

QString JoyButtonSlot::getXmlName()
{
    return this->xmlName;
}

QString JoyButtonSlot::getSlotString()
{
    QString newlabel;

    if (deviceCode > 0)
    {
        if (mode == JoyButtonSlot::JoyKeyboard)
        {
            newlabel = newlabel.append(keysymToKey(deviceCode).toUpper());
        }
        else if (mode == JoyButtonSlot::JoyMouseButton)
        {
            newlabel.append(tr("Mouse")).append(" ");
            switch (deviceCode)
            {
                case 1:
                    newlabel.append(tr("LB"));
                    break;
                case 2:
                    newlabel.append(tr("MB"));
                    break;
                case 3:
                    newlabel.append(tr("RB"));
                    break;
#ifdef Q_OS_WIN
                case 8:
                    newlabel.append(tr("B4"));
                    break;
                case 9:
                    newlabel.append(tr("B5"));
                    break;
#endif
                default:
                    newlabel.append(QString::number(deviceCode));
                    break;
            }
        }
        else if (mode == JoyButtonSlot::JoyMouseMovement)
        {
            newlabel.append(movementString());
        }
        else if (mode == JoyButtonSlot::JoyPause)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Pause")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyButtonSlot::JoyHold)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Hold")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            newlabel.append(tr("Cycle"));
        }
        else if (mode == JoyButtonSlot::JoyDistance)
        {
            QString temp(tr("Distance"));
            temp.append(" ").append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);
        }
        else if (mode == JoyButtonSlot::JoyRelease)
        {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Release")).append(" ");
            if (minutes > 0)
            {
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
            }

            newlabel.append(QString("%1.%2")
                    .arg(seconds, 2, 10, QChar('0'))
                    .arg(hundredths, 2, 10, QChar('0')));
        }
        else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
        {
            QString temp;
            temp.append(tr("Mouse Mod")).append(" ");
            temp.append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);
        }
    }
    else
    {
        newlabel = newlabel.append(tr("[NO KEY]"));
    }

    return newlabel;
}
