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

#include "joybuttonslot.h"

#include "messagehandler.h"
#include "inputdevice.h"
#include "antkeymapper.h"
#include "event.h"

#include <QDebug>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const int JoyButtonSlot::JOYSPEED = 20;
const QString JoyButtonSlot::xmlName = "slot";
const int JoyButtonSlot::MAXTEXTENTRYDISPLAYLENGTH = 40;

JoyButtonSlot::JoyButtonSlot(QObject *parent) :
    QObject(parent),
    extraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceCode = 0;
    m_mode = JoyKeyboard;
    m_distance = 0.0;
    previousDistance = 0.0;
    qkeyaliasCode = 0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent) :
    QObject(parent),
    extraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceCode = 0;
    qkeyaliasCode = 0;

    if (code > 0)
    {
        deviceCode = code;
    }

    m_mode = mode;
    m_distance = 0.0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(int code, int alias, JoySlotInputAction mode, QObject *parent) :
    QObject(parent),
    extraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceCode = 0;
    qkeyaliasCode = 0;

    if (code > 0)
    {
        deviceCode = code;
    }

    if (alias > 0)
    {
        qkeyaliasCode = alias;
    }

    m_mode = mode;
    m_distance = 0.0;
    easingActive = false;
}

JoyButtonSlot::JoyButtonSlot(JoyButtonSlot *slot, QObject *parent) :
    QObject(parent),
    extraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceCode = slot->deviceCode;
    qkeyaliasCode = slot->qkeyaliasCode;
    m_mode = slot->m_mode;
    m_distance = slot->m_distance;
    easingActive = false;
    m_textData = slot->getTextData();
    extraData = slot->getExtraData();
}

JoyButtonSlot::JoyButtonSlot(QString text, JoySlotInputAction mode, QObject *parent) :
    QObject(parent),
    extraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceCode = 0;
    qkeyaliasCode = 0;
    m_mode = mode;
    m_distance = 0.0;
    easingActive = false;
    if ((mode == JoyLoadProfile) ||
        (mode == JoyTextEntry) ||
        (mode == JoyExecute))
    {
        m_textData = text;
    }
}

void JoyButtonSlot::setSlotCode(int code)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

void JoyButtonSlot::setSlotCode(int code, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((m_mode == JoyButtonSlot::JoyKeyboard) && (code > 0))
    {
        deviceCode = code;
        qkeyaliasCode = alias;
    }
    else if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

int JoyButtonSlot::getSlotCodeAlias()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return qkeyaliasCode;
}

int JoyButtonSlot::getSlotCode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return deviceCode;
}

void JoyButtonSlot::setSlotMode(JoySlotInputAction selectedMode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_mode = selectedMode;
}

JoyButtonSlot::JoySlotInputAction JoyButtonSlot::getSlotMode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_mode;
}

QString JoyButtonSlot::movementString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = QString();

    if (m_mode == JoyMouseMovement)
    {
        newlabel.append(trUtf8("Mouse")).append(" ");

        switch(deviceCode)
        {

            case 1:
                newlabel.append(trUtf8("Up"));
                break;

            case 2:
                newlabel.append(trUtf8("Down"));
                break;

            case 3:
                newlabel.append(trUtf8("Left"));
                break;

            case 4:
                newlabel.append(trUtf8("Right"));
                break;

        }
    }

    return newlabel;
}

void JoyButtonSlot::setDistance(double distance)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_distance = distance;
}

double JoyButtonSlot::getMouseDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_distance;
}

QElapsedTimer* JoyButtonSlot::getMouseInterval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &mouseInterval;
}

void JoyButtonSlot::restartMouseInterval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseInterval.restart();
}

void JoyButtonSlot::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "slot"))
    {
        QString profile = QString();
        QString tempStringData = QString();
        QString extraStringData = QString();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "slot")))
        {
            if ((xml->name() == "code") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                bool ok = false;
                int tempchoice = temptext.toInt(&ok, 0);
                if (ok)
                {
                    this->setSlotCode(tempchoice);
                }
            }
            else if ((xml->name() == "profile") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                profile = temptext;
            }
            else if ((xml->name() == "text") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if ((xml->name() == "path") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if ((xml->name() == "arguments") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                extraStringData = temptext;
            }
            else if ((xml->name() == "mode") && xml->isStartElement())
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
                else if (temptext == "keypress")
                {
                    this->setSlotMode(JoyKeyPress);
                }
                else if (temptext == "delay")
                {
                    this->setSlotMode(JoyDelay);
                }
                else if (temptext == "loadprofile")
                {
                    this->setSlotMode(JoyLoadProfile);
                }
                else if (temptext == "setchange")
                {
                    this->setSlotMode(JoySetChange);
                }
                else if (temptext == "textentry")
                {
                    this->setSlotMode(JoyTextEntry);
                }
                else if (temptext == "execute")
                {
                    this->setSlotMode(JoyExecute);
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
            int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(this->getSlotCode());
            int tempkey = this->getSlotCode();

            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                this->setSlotCode(virtualkey, tempkey);
            }
            else if (this->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                int temp = this->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                this->setSlotCode(temp);
            }
        }
        else if ((this->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !profile.isEmpty())
        {
            QFileInfo profileInfo(profile);
            if (!profileInfo.exists() || !((profileInfo.suffix() == "amgp") || (profileInfo.suffix() == "xml")))
            {
                this->setTextData("");
            }
            else
            {
                this->setTextData(profile);
            }
        }
        else if (this->getSlotMode() == JoySetChange)
        {
            if (!(this->getSlotCode() >= 0) && !(this->getSlotCode() < InputDevice::NUMBER_JOYSETS))
            {
                this->setSlotCode(-1);
            }
        }
        else if ((this->getSlotMode() == JoyTextEntry) && !tempStringData.isEmpty())
        {
            this->setTextData(tempStringData);
        }
        else if ((this->getSlotMode() == JoyExecute) && !tempStringData.isEmpty())
        {
            QFileInfo tempFile(tempStringData);
            if (tempFile.exists() && tempFile.isExecutable())
            {
                this->setTextData(tempStringData);
                if (!extraStringData.isEmpty())
                {
                    this->setExtraData(QVariant(extraStringData));
                }
            }
        }
    }
}

void JoyButtonSlot::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(getXmlName());

    if (m_mode == JoyKeyboard)
    {
        int basekey = AntKeyMapper::getInstance()->returnQtKey(deviceCode);
        int qtkey = this->getSlotCodeAlias();
        if ((qtkey > 0) || (basekey > 0))
        {
            // Did not add an alias to slot. If a possible Qt key value
            // was found, use it.
            if ((qtkey == 0) && (basekey > 0))
            {
                qtkey = basekey;
            }

            // Found a valid abstract keysym.
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "ANT KEY: " << QString::number(qtkey, 16);
            #endif

            xml->writeTextElement("code", QString("0x%1").arg(qtkey, 0, 16));
        }
        else if (deviceCode > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            int tempkey = deviceCode | QtKeyMapperBase::nativeKeyPrefix;

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "ANT KEY: " << QString::number(tempkey, 16);
            #endif

            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }
    }
    else if ((m_mode == JoyLoadProfile) && !m_textData.isEmpty())
    {
        xml->writeTextElement("profile", m_textData);
    }
    else if ((m_mode == JoyTextEntry) && !m_textData.isEmpty())
    {
        xml->writeTextElement("text", m_textData);
    }
    else if ((m_mode == JoyExecute) && !m_textData.isEmpty())
    {
        xml->writeTextElement("path", m_textData);
        if (!extraData.isNull() && extraData.canConvert<QString>())
        {
            xml->writeTextElement("arguments", extraData.toString());
        }
    }
    else
    {
        xml->writeTextElement("code", QString::number(deviceCode));
    }

    xml->writeStartElement("mode");

    switch(m_mode)
    {

        case JoyKeyboard:
            xml->writeCharacters("keyboard");
            break;

        case JoyMouseButton:
            xml->writeCharacters("mousebutton");
            break;

        case JoyMouseMovement:
            xml->writeCharacters("mousemovement");
            break;

        case JoyPause:
            xml->writeCharacters("pause");
            break;

        case JoyHold:
            xml->writeCharacters("hold");
            break;

        case JoyCycle:
            xml->writeCharacters("cycle");
            break;

        case JoyDistance:
            xml->writeCharacters("distance");
            break;

        case JoyRelease:
            xml->writeCharacters("release");
            break;

        case JoyMouseSpeedMod:
            xml->writeCharacters("mousespeedmod");
            break;

        case JoyKeyPress:
            xml->writeCharacters("keypress");
            break;

        case JoyDelay:
            xml->writeCharacters("delay");
            break;

        case JoyLoadProfile:
            xml->writeCharacters("loadprofile");
            break;

        case JoySetChange:
            xml->writeCharacters("setchange");
            break;

        case JoyTextEntry:
            xml->writeCharacters("textentry");
            break;

        case JoyExecute:
            xml->writeCharacters("execute");
            break;

    }


    xml->writeEndElement();

    xml->writeEndElement();
}

QString JoyButtonSlot::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->xmlName;
}

QString JoyButtonSlot::getSlotString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = QString();

    if (deviceCode >= 0)
    {
        switch(m_mode)
        {

            case JoyButtonSlot::JoyKeyboard:
            {

                int tempDeviceCode = deviceCode;
    #ifdef Q_OS_WIN
                QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
                if (nativeWinKeyMapper)
                {
                    tempDeviceCode = nativeWinKeyMapper->returnVirtualKey(qkeyaliasCode);
                }
    #endif
                newlabel = newlabel.append(keysymToKeyString(tempDeviceCode, qkeyaliasCode).toUpper());
            break;

            }
            case JoyButtonSlot::JoyMouseButton:
            {
                newlabel.append(trUtf8("Mouse")).append(" ");
                switch (deviceCode)
                {
                    case 1:
                        newlabel.append(trUtf8("LB"));
                        break;
                    case 2:
                        newlabel.append(trUtf8("MB"));
                        break;
                    case 3:
                        newlabel.append(trUtf8("RB"));
                        break;
    #ifdef Q_OS_WIN
                    case 8:
                        newlabel.append(trUtf8("B4"));
                        break;
                    case 9:
                        newlabel.append(trUtf8("B5"));
                        break;
    #endif
                    default:
                        newlabel.append(QString::number(deviceCode));
                        break;
                }

                break;
            }
            case JoyMouseMovement:
            {
                newlabel.append(movementString());
                break;
            }
            case JoyPause:
            {
                int minutes = deviceCode / 1000 / 60;
                int seconds = (deviceCode / 1000 % 60);
                int hundredths = deviceCode % 1000 / 10;

                newlabel.append(trUtf8("Pause")).append(" ");
                if (minutes > 0)
                {
                    newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
                }

                newlabel.append(QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(hundredths, 2, 10, QChar('0')));

                break;
            }
            case JoyHold:
            {
                int minutes = deviceCode / 1000 / 60;
                int seconds = (deviceCode / 1000 % 60);
                int hundredths = deviceCode % 1000 / 10;

                newlabel.append(trUtf8("Hold")).append(" ");
                if (minutes > 0)
                {
                    newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
                }

                newlabel.append(QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(hundredths, 2, 10, QChar('0')));

                break;
            }
            case JoyButtonSlot::JoyCycle:
            {
                newlabel.append(trUtf8("Cycle"));
                break;
            }
            case JoyDistance:
            {
                QString temp(trUtf8("Distance"));
                temp.append(" ").append(QString::number(deviceCode).append("%"));
                newlabel.append(temp);

                break;
            }
            case JoyRelease:
            {
                int minutes = deviceCode / 1000 / 60;
                int seconds = (deviceCode / 1000 % 60);
                int hundredths = deviceCode % 1000 / 10;

                newlabel.append(trUtf8("Release")).append(" ");
                if (minutes > 0)
                {
                    newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
                }

                newlabel.append(QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(hundredths, 2, 10, QChar('0')));

                break;
            }
            case JoyMouseSpeedMod:
            {
                QString temp = QString();
                temp.append(trUtf8("Mouse Mod")).append(" ");
                temp.append(QString::number(deviceCode).append("%"));
                newlabel.append(temp);

                break;
            }
            case JoyKeyPress:
            {
                int minutes = deviceCode / 1000 / 60;
                int seconds = (deviceCode / 1000 % 60);
                int hundredths = deviceCode % 1000 / 10;

                QString temp = QString();
                temp.append(trUtf8("Press Time")).append(" ");
                if (minutes > 0)
                {
                    temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
                }

                temp.append(QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(hundredths, 2, 10, QChar('0')));

                newlabel.append(temp);

                break;
            }
            case JoyDelay:
            {
                int minutes = deviceCode / 1000 / 60;
                int seconds = (deviceCode / 1000 % 60);
                int hundredths = deviceCode % 1000 / 10;

                QString temp = QString();
                temp.append(trUtf8("Delay")).append(" ");
                if (minutes > 0)
                {
                    temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));
                }

                temp.append(QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(hundredths, 2, 10, QChar('0')));

                newlabel.append(temp);

                break;
            }
            case JoyLoadProfile:
            {
                if (!m_textData.isEmpty())
                {
                    QFileInfo profileInfo(m_textData);
                    QString temp = QString();
                    temp.append(trUtf8("Load %1").arg(PadderCommon::getProfileName(profileInfo)));
                    newlabel.append(temp);
                }

                break;
            }
            case JoySetChange:
            {
                newlabel.append(trUtf8("Set Change %1").arg(deviceCode+1));

                break;
            }
            case JoyTextEntry:
            {
                QString temp = m_textData;
                if (temp.length() > MAXTEXTENTRYDISPLAYLENGTH)
                {
                    temp.truncate(MAXTEXTENTRYDISPLAYLENGTH - 3);
                    temp.append("...");
                }
                newlabel.append(trUtf8("[Text] %1").arg(temp));

                break;
            }
            case JoyExecute:
            {
                QString temp = QString();
                if (!m_textData.isEmpty())
                {
                    QFileInfo tempFileInfo(m_textData);
                    temp.append(tempFileInfo.fileName());
                }

                newlabel.append(trUtf8("[Exec] %1").arg(temp));

                break;
            }
        }
    }
    else
    {
        newlabel = newlabel.append(trUtf8("[NO KEY]"));
    }

    return newlabel;
}

void JoyButtonSlot::setPreviousDistance(double distance)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    previousDistance = distance;
}

double JoyButtonSlot::getPreviousDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return previousDistance;
}

bool JoyButtonSlot::isModifierKey()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool modifier = false;
    if ((m_mode == JoyKeyboard) && AntKeyMapper::getInstance()->isModifierKey(qkeyaliasCode))
    {
        modifier = true;
    }

    return modifier;
}

bool JoyButtonSlot::isEasingActive()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return easingActive;
}

void JoyButtonSlot::setEasingStatus(bool isActive)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    easingActive = isActive;
}

QTime* JoyButtonSlot::getEasingTime()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &easingTime;
}

void JoyButtonSlot::setTextData(QString textData)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_textData = textData;
}

QString JoyButtonSlot::getTextData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_textData;
}

void JoyButtonSlot::setExtraData(QVariant data)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->extraData = data;
}

QVariant JoyButtonSlot::getExtraData()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return extraData;
}

bool JoyButtonSlot::isValidSlot()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = true;
    switch (m_mode)
    {
        case JoyLoadProfile:
        case JoyTextEntry:
        case JoyExecute:
        {
            if (m_textData.isEmpty())
            {
                result = false;
            }

            break;
        }
        case JoySetChange:
        {
            if (deviceCode < 0)
            {
                result = false;
            }

            break;
        }
    case JoyKeyboard:
    {
        break;
    }
    case JoyMouseButton:
    {
        break;
    }
    case JoyMouseMovement:
    {
        break;
    }
    case JoyPause:
    {
        break;
    }
    case JoyHold:
    {
        break;
    }
    case JoyCycle:
    {
        break;
    }
    case JoyDistance:
    {
        break;
    }
    case JoyRelease:
    {
        break;
    }
    case JoyMouseSpeedMod:
    {
        break;
    }
    case JoyKeyPress:
    {
        break;
    }
    case JoyDelay:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    return result;
}
