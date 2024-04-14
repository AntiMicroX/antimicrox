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

#include "joyaxisxml.h"
#include "haptictriggerps5.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "joybuttontypes/joyaxisbutton.h"
#include "xml/joybuttonxml.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

JoyAxisXml::JoyAxisXml(JoyAxis *axis, QObject *parent)
    : QObject(parent)
{
    m_joyAxis = axis;
    joyButtonXmlNAxis = new JoyButtonXml(axis->getNAxisButton());
    joyButtonXmlPAxis = new JoyButtonXml(axis->getPAxisButton());
}

JoyAxisXml::~JoyAxisXml()
{
    if (!joyButtonXmlNAxis.isNull())
        joyButtonXmlNAxis->deleteLater();
    if (!joyButtonXmlPAxis.isNull())
        joyButtonXmlPAxis->deleteLater();
}

void JoyAxisXml::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == m_joyAxis->getXmlName()))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != m_joyAxis->getXmlName())))
        {
            bool found = readMainConfig(xml);

            if (!found && (xml->name().toString() == m_joyAxis->getNAxisButton()->getXmlName()) && xml->isStartElement())
            {
                found = true;
                readButtonConfig(xml);
            }

            if (!found)
                xml->skipCurrentElement();

            xml->readNextStartElement();
        }
    }
}

void JoyAxisXml::writeConfig(QXmlStreamWriter *xml)
{
    bool currentlyDefault = m_joyAxis->isDefault();

    xml->writeStartElement(m_joyAxis->getXmlName());
    xml->writeAttribute("index", QString::number(m_joyAxis->getRealJoyIndex()));

    if (!currentlyDefault)
    {
        if (m_joyAxis->getDeadZone() != GlobalVariables::JoyAxis::AXISDEADZONE)
            xml->writeTextElement("deadZone", QString::number(m_joyAxis->getDeadZone()));

        if (m_joyAxis->getMaxZoneValue() != GlobalVariables::JoyAxis::AXISMAXZONE)
            xml->writeTextElement("maxZone", QString::number(m_joyAxis->getMaxZoneValue()));
    }

    xml->writeStartElement("throttle");

    switch (m_joyAxis->getThrottle())
    {
    case -2:
        xml->writeCharacters("negativehalf");
        break;

    case -1:
        xml->writeCharacters("negative");
        break;

    case 0:
        xml->writeCharacters("normal");
        break;

    case 1:
        xml->writeCharacters("positive");
        break;

    case 2:
        xml->writeCharacters("positivehalf");
        break;
    }

    xml->writeEndElement();

    if (m_joyAxis->hasHapticTrigger())
    {
        HapticTriggerPs5 *haptic = m_joyAxis->getHapticTrigger();
        xml->writeTextElement("hapticTrigger", HapticTriggerPs5::to_string(haptic->get_mode()));
    }

    if (!currentlyDefault)
    {
        joyButtonXmlNAxis->writeConfig(xml);
        joyButtonXmlPAxis->writeConfig(xml);
    }

    xml->writeEndElement();
}

bool JoyAxisXml::readMainConfig(QXmlStreamReader *xml)
{
    bool found = false;

    if ((xml->name().toString() == "deadZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        qDebug() << "From xml config dead zone is: " << tempchoice;

        m_joyAxis->setDeadZone(tempchoice);
    } else if ((xml->name().toString() == "maxZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        qDebug() << "From xml config max zone is: " << tempchoice;

        m_joyAxis->setMaxZoneValue(tempchoice);
    } else if ((xml->name().toString() == "throttle") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        qDebug() << "From xml config throttle name is: " << temptext;

        if (temptext == "negativehalf")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NegativeHalfThrottle));
        } else if (temptext == "negative")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NegativeThrottle));
        } else if (temptext == "normal")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NormalThrottle));
        } else if (temptext == "positive")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::PositiveThrottle));
        } else if (temptext == "positivehalf")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::PositiveHalfThrottle));
        }

        InputDevice *device = m_joyAxis->getParentSet()->getInputDevice();

        if (!device->hasCalibrationThrottle(m_joyAxis->getIndex()))
        {
            device->setCalibrationStatus(m_joyAxis->getIndex(),
                                         static_cast<JoyAxis::ThrottleTypes>(m_joyAxis->getThrottle()));
        }

        m_joyAxis->setCurrentRawValue(m_joyAxis->getCurrentThrottledDeadValue());
        m_joyAxis->updateCurrentThrottledValue(m_joyAxis->calculateThrottledValue(m_joyAxis->getCurrentRawValue()));
    } else if ((xml->name().toString() == "hapticTrigger") && xml->isStartElement())
    {
        found = true;
        m_joyAxis->setHapticTriggerMode(HapticTriggerPs5::from_string(xml->readElementText()));
    }

    return found;
}

bool JoyAxisXml::readButtonConfig(QXmlStreamReader *xml)
{
    bool found = false;

    int index_local = xml->attributes().value("index").toString().toInt();

    if (index_local == 1)
    {
        found = true;
        joyButtonXmlNAxis->readConfig(xml);
    } else if (index_local == 2)
    {
        found = true;
        joyButtonXmlPAxis->readConfig(xml);
    }

    return found;
}
