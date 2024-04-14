/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "joybuttonslotxml.h"
#include "antkeymapper.h"
#include "globalvariables.h"
#include "joybuttonslot.h"

#include <QDebug>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

int JoyButtonSlotXml::timeoutWrite = 5000;
int JoyButtonSlotXml::timeoutRead = 5000;

JoyButtonSlotXml::JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent)
    : QObject(parent)
    , m_joyBtnSlot(joyBtnSlot)
{
}

void JoyButtonSlotXml::readConfig(QXmlStreamReader *xml)
{ // QWriteLocker tempLocker(&xmlLock);
    std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
    t1 = std::chrono::high_resolution_clock::now();

    bool result = xmlLock.tryLockForWrite();

    if (!result && timeoutWrite > 0)
        xmlLock.tryLockForWrite(timeoutWrite);

    qDebug() << "START OF READ CONFIG NAME: " << xml->name().toString();

    if (xml->isStartElement() && (xml->name().toString() == "slot"))
    {
        QString profile = QString();
        QString tempStringData = QString();
        QString extraStringData = QString();

        xml->readNextStartElement();

        qDebug() << "NEXT TO THE START TAG NAME: " << xml->name().toString();

        // so it must be JoyMix
        if (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() == "slots")))
        {
            qDebug() << "Detected mix slots";

            QString slotMixString = QString();
            bool firstTimePlus = true;

            xml->readNextStartElement();

            int i = 0;

            while (xml->name().toString() == "slot")
            {
                qDebug() << "Found mini slot in xml file";

                xml->readNextStartElement(); // skip to minislot within slots list

                qDebug() << "Now xml name after read next is: " << xml->name().toString();

                // we don't want to add empty slot to minislots
                // skip again and check name of next tag
                if (xml->name().toString() == "slot")
                    xml->readNextStartElement();

                // if reached the end of mini slots, read next elem, that should be mode JoyMix and break loop
                if (xml->name().toString() == "slots")
                {
                    xml->readNextStartElement();
                    break;
                }

                qDebug() << "And now xml name after read next is: " << xml->name().toString();

                JoyButtonSlot *minislot = new JoyButtonSlot();

                readEachSlot(xml, minislot, profile, tempStringData, extraStringData);

                i++;
                m_joyBtnSlot->appendMiniSlot<JoyButtonSlot *>(minislot);

                if (!firstTimePlus)
                    slotMixString.append('+');
                firstTimePlus = false;

                slotMixString.append(minislot->getSlotString());

                qDebug() << "Slot mix string now is named: " << slotMixString;
                qDebug() << "Added " << i << " minislots to current slot from xml file";
                qDebug() << "Added mini slot string and mode and code: " << minislot->getSlotString() << " and "
                         << minislot->getSlotMode() << " and " << minislot->getSlotCode();

                qDebug() << "After readEachSlot for JoyMix now should be \"slot\" again or \"mode\": "
                         << xml->name().toString();
                qDebug() << "It it start element? :" << (xml->isStartElement() ? "yes" : "no");
            }

            i = 0;

            if (xml->name().toString() == "mode" && xml->readElementText() == "mix")
            {
                qDebug() << "slot text data for joy mix is: " << slotMixString;

                m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMix);
                m_joyBtnSlot->setTextData(slotMixString);
                m_joyBtnSlot->setSlotCode(-1);

                profile = QString();
                tempStringData = QString();
                extraStringData = QString();
                slotMixString = QString();

                xml->readNextStartElement();
            }
        } else
        {
            readEachSlot(xml, m_joyBtnSlot, profile, tempStringData, extraStringData);

            qDebug() << "Detected simple slot: " << m_joyBtnSlot->getSlotString();
        }
    }

    xmlLock.unlock();

    t2 = std::chrono::high_resolution_clock::now();

    if (timeoutRead == 3000)
        timeoutRead = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

void JoyButtonSlotXml::readEachSlot(QXmlStreamReader *xml, JoyButtonSlot *joyBtnSlot, QString &profile,
                                    QString &tempStringData, QString &extraStringData)
{
    while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "slot")))
    {
        if ((xml->name().toString() == "code") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();
            bool ok = false;
            int tempchoice = temptext.toInt(&ok, 0);

            if (ok)
                joyBtnSlot->setSlotCode(tempchoice);
        } else if ((xml->name().toString() == "profile") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();
            profile = temptext;
        } else if ((xml->name().toString() == "text") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();
            tempStringData = temptext;
        } else if ((xml->name().toString() == "path") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();
            tempStringData = temptext;
        } else if ((xml->name().toString() == "arguments") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();
            extraStringData = temptext;
        } else if ((xml->name().toString() == "mode") && xml->isStartElement())
        {
            QString temptext = xml->readElementText();

            if (temptext == "keyboard")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyKeyboard);
            } else if (temptext == "mousebutton")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseButton);
            } else if (temptext == "mousemovement")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseMovement);
            } else if (temptext == "pause")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyPause);
            } else if (temptext == "hold")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyHold);
            } else if (temptext == "cycle")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyCycle);
            } else if (temptext == "distance")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyDistance);
            } else if (temptext == "release")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyRelease);
            } else if (temptext == "mousespeedmod")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseSpeedMod);
            } else if (temptext == "keypress")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyKeyPress);
            } else if (temptext == "delay")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyDelay);
            } else if (temptext == "loadprofile")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyLoadProfile);
            } else if (temptext == "setchange")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoySetChange);
            } else if (temptext == "textentry")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyTextEntry);
            } else if (temptext == "execute")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyExecute);
            } else if (temptext == "mix")
            {
                joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMix);
            }
        } else
        {
            xml->skipCurrentElement();
        }

        xml->readNextStartElement();
    }

    setSlotData(joyBtnSlot, profile, tempStringData, extraStringData);
}

void JoyButtonSlotXml::setSlotData(JoyButtonSlot *joyBtnSlot, QString profile, QString tempStringData,
                                   QString extraStringData)
{
    if (joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
    {
        int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(joyBtnSlot->getSlotCode());
        int tempkey = joyBtnSlot->getSlotCode();

        if (virtualkey)
        {
            // Mapping found a valid native keysym.
            joyBtnSlot->setSlotCode(virtualkey, tempkey);
        } else if (joyBtnSlot->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
        {
            // Value is in the native key range. Remove prefix and use
            // new value as a native keysym.
            int temp = joyBtnSlot->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
            joyBtnSlot->setSlotCode(temp);
        }
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !profile.isEmpty())
    {
        QFileInfo profileInfo(profile);

        if (!profileInfo.exists() || !((profileInfo.suffix() == "amgp") || (profileInfo.suffix() == "xml")))
        {
            joyBtnSlot->setTextData("");
        } else
        {
            joyBtnSlot->setTextData(profile);
        }
    } else if (joyBtnSlot->getSlotMode() == JoyButtonSlot::JoySetChange && !(joyBtnSlot->getSlotCode() >= 0) &&
               !(joyBtnSlot->getSlotCode() < GlobalVariables::InputDevice::NUMBER_JOYSETS))
    {
        joyBtnSlot->setSlotCode(-1);
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !tempStringData.isEmpty())
    {
        joyBtnSlot->setTextData(tempStringData);
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !tempStringData.isEmpty())
    {
        QFileInfo tempFile(tempStringData);

        if (tempFile.exists())
        {
            joyBtnSlot->setTextData(tempStringData);

            if (!extraStringData.isEmpty())
                joyBtnSlot->setExtraData(QVariant(extraStringData));
        }
    }
}

void JoyButtonSlotXml::writeConfig(QXmlStreamWriter *xml)
{ // QReadLocker tempLocker(&xmlLock);
    std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
    t1 = std::chrono::high_resolution_clock::now();

    bool result = xmlLock.tryLockForRead();

    if (!result && timeoutRead > 0)
        xmlLock.tryLockForRead(timeoutRead);

    xml->writeStartElement(m_joyBtnSlot->getXmlName());

    if (m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyMix)
    {
        qDebug() << "write JoyMix slot in xml file";

        xml->writeStartElement("slots");

        QListIterator<JoyButtonSlot *> iterMini(*m_joyBtnSlot->getMixSlots());

        while (iterMini.hasNext())
        {
            JoyButtonSlot *minislot = iterMini.next();
            qDebug() << "write minislot: " << minislot->getSlotString();

            xml->writeStartElement(m_joyBtnSlot->getXmlName());
            writeEachSlot(xml, minislot);
            xml->writeEndElement();
        }

        xml->writeEndElement();

        xml->writeStartElement("mode");
        xml->writeCharacters("mix");
        xml->writeEndElement();
    } else
    {
        writeEachSlot(xml, m_joyBtnSlot);
    }

    xml->writeEndElement();
    xmlLock.unlock();

    t2 = std::chrono::high_resolution_clock::now();

    if (timeoutWrite == 3000)
        timeoutWrite = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

void JoyButtonSlotXml::writeEachSlot(QXmlStreamWriter *xml, JoyButtonSlot *joyBtnSlot)
{
    qDebug() << "slot mode for " << joyBtnSlot->getSlotString() << " is " << joyBtnSlot->getSlotMode();

    if (joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
    {
        int basekey = AntKeyMapper::getInstance()->returnQtKey(joyBtnSlot->getSlotCode());
        int qtkey = joyBtnSlot->getSlotCodeAlias();

        if ((qtkey > 0) || (basekey > 0))
        {
            // Did not add an alias to slot. If a possible Qt key value
            // was found, use it.
            if ((qtkey == 0) && (basekey > 0))
                qtkey = basekey;

            // Found a valid abstract keysym.
            qDebug() << "ANT KEY: " << QString::number(qtkey, 16);

            xml->writeTextElement("code", QString("0x%1").arg(qtkey, 0, 16));
        } else if (joyBtnSlot->getSlotCode() > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            int tempkey = joyBtnSlot->getSlotCode() | QtKeyMapperBase::nativeKeyPrefix;

            qDebug() << "ANT KEY: " << QString::number(tempkey, 16);

            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("profile", joyBtnSlot->getTextData());
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("text", joyBtnSlot->getTextData());
    } else if ((joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("path", joyBtnSlot->getTextData());

        if (!joyBtnSlot->getExtraData().isNull() && joyBtnSlot->getExtraData().canConvert<QString>())
        {
            xml->writeTextElement("arguments", joyBtnSlot->getExtraData().toString());
        }
    } else
    {
        xml->writeTextElement("code", QString::number(joyBtnSlot->getSlotCode()));
    }

    qDebug() << "write mode for " << joyBtnSlot->getSlotString();
    xml->writeStartElement("mode");

    switch (joyBtnSlot->getSlotMode())
    {
    case JoyButtonSlot::JoyKeyboard:
        xml->writeCharacters("keyboard");
        break;

    case JoyButtonSlot::JoyMouseButton:
        xml->writeCharacters("mousebutton");
        break;

    case JoyButtonSlot::JoyMouseMovement:
        xml->writeCharacters("mousemovement");
        break;

    case JoyButtonSlot::JoyPause:
        xml->writeCharacters("pause");
        break;

    case JoyButtonSlot::JoyHold:
        xml->writeCharacters("hold");
        break;

    case JoyButtonSlot::JoyCycle:
        xml->writeCharacters("cycle");
        break;

    case JoyButtonSlot::JoyDistance:
        xml->writeCharacters("distance");
        break;

    case JoyButtonSlot::JoyRelease:
        xml->writeCharacters("release");
        break;

    case JoyButtonSlot::JoyMouseSpeedMod:
        xml->writeCharacters("mousespeedmod");
        break;

    case JoyButtonSlot::JoyKeyPress:
        xml->writeCharacters("keypress");
        break;

    case JoyButtonSlot::JoyDelay:
        xml->writeCharacters("delay");
        break;

    case JoyButtonSlot::JoyLoadProfile:
        xml->writeCharacters("loadprofile");
        break;

    case JoyButtonSlot::JoySetChange:
        xml->writeCharacters("setchange");
        break;

    case JoyButtonSlot::JoyTextEntry:
        xml->writeCharacters("textentry");
        break;

    case JoyButtonSlot::JoyExecute:
        xml->writeCharacters("execute");
        break;

    case JoyButtonSlot::JoyMix:
        xml->writeCharacters("mix");
        break;
    }

    xml->writeEndElement();
}
