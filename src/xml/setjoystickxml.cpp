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

#include "setjoystickxml.h"
#include "xml/joyaxisxml.h"
#include "xml/joybuttonxml.h"
#include "xml/joydpadxml.h"
#include <iostream>
#include <memory>

#include "joyaxis.h"
#include "joybuttontypes/joybutton.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joysensor.h"
#include "vdpad.h"

#include "setjoystick.h"

#include <QDebug>
#include <QFuture>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtConcurrent/QtConcurrent>

SetJoystickXml::SetJoystickXml(SetJoystick *setJoystick, QObject *parent)
    : QObject(parent)
    , m_setJoystick(setJoystick)
{
}

/**
 * @brief Deserializes the given XML stream into a SetJoystick object
 * @param[in] xml The XML stream to read from
 */
void SetJoystickXml::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == "set"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "set")))
        {
            if ((xml->name().toString() == "button") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = m_setJoystick->getJoyButton(index - 1);
                joyButtonXml = new JoyButtonXml(button);

                if (button != nullptr)
                    joyButtonXml->readConfig(xml);
                else
                    xml->skipCurrentElement();
            } else if ((xml->name().toString() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = m_setJoystick->getJoyAxis(index - 1);
                joyAxisXml = new JoyAxisXml(axis);

                if (axis != nullptr)
                    joyAxisXml->readConfig(xml);
                else
                    xml->skipCurrentElement();
            } else if ((xml->name().toString() == "dpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = m_setJoystick->getJoyDPad(index - 1);
                JoyDPadXml<JoyDPad> *joydpadXml = new JoyDPadXml<JoyDPad>(dpad);

                if (dpad != nullptr)
                    joydpadXml->readConfig(xml);
                else
                    xml->skipCurrentElement();
            } else if ((xml->name().toString() == "stick") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = m_setJoystick->getJoyStick(stickIndex);

                    if (stick != nullptr)
                        stick->readConfig(xml);
                    else
                        xml->skipCurrentElement();
                } else
                {
                    xml->skipCurrentElement();
                }
            } else if ((xml->name().toString() == "sensor") && xml->isStartElement())
            {
                int type = xml->attributes().value("type").toString().toInt();
                JoySensor *sensor = m_setJoystick->getSensor(static_cast<JoySensorType>(type));

                if (sensor != nullptr)
                    sensor->readConfig(xml);
                else
                    xml->skipCurrentElement();
            } else if ((xml->name().toString() == "vdpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = m_setJoystick->getVDPad(index - 1);
                JoyDPadXml<VDPad> *joydpadXml = new JoyDPadXml<VDPad>(vdpad);

                if (vdpad != nullptr)
                    joydpadXml->readConfig(xml);
                else
                    xml->skipCurrentElement();
            } else if ((xml->name().toString() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (!temptext.isEmpty())
                    m_setJoystick->setName(temptext);
            } else
            {
                xml->skipCurrentElement(); // If none of the above, skip the element
            }

            xml->readNextStartElement();
        }
    }
}

/**
 * @brief Serializes a SetJoystick object into the the given XML stream
 * @param[in,out] xml The XML stream to write to
 */
void SetJoystickXml::writeConfig(QXmlStreamWriter *xml)
{
    if (!m_setJoystick->isSetEmpty())
    {
        xml->writeStartElement("set");
        xml->writeAttribute("index", QString::number(m_setJoystick->getIndex() + 1));

        if (!m_setJoystick->getName().isEmpty())
            xml->writeTextElement("name", m_setJoystick->getName());

        QList<JoyControlStick *> sticksList = m_setJoystick->getSticks().values();
        QListIterator<JoyControlStick *> i(sticksList);
        while (i.hasNext())
            i.next()->writeConfig(xml);

        auto sensors = m_setJoystick->getSensors();
        for (const auto &sensor : sensors)
            sensor->writeConfig(xml);

        QList<VDPad *> vdpadsList = m_setJoystick->getVdpads().values();
        QListIterator<VDPad *> vdpad(vdpadsList);
        while (vdpad.hasNext())
        {
            JoyDPadXml<VDPad> *joydpadXml = new JoyDPadXml<VDPad>(vdpad.next());
            joydpadXml->writeConfig(xml);
            delete joydpadXml;
            joydpadXml = nullptr;
        }

        QList<JoyAxis *> axesList = m_setJoystick->getAxes()->values();
        QListIterator<JoyAxis *> axis(axesList);
        while (axis.hasNext())
        {
            JoyAxis *axisCur = axis.next();
            JoyAxisXml *joyAxisXml = new JoyAxisXml(axisCur);

            if (!axisCur->isPartControlStick() && axisCur->hasControlOfButtons())
            {
                joyAxisXml->writeConfig(xml);
            }

            delete joyAxisXml;
            joyAxisXml = nullptr;
        }

        QList<JoyDPad *> dpadsList = m_setJoystick->getHats().values();
        QListIterator<JoyDPad *> dpad(dpadsList);
        while (dpad.hasNext())
        {
            JoyDPadXml<JoyDPad> *joydpadXml = new JoyDPadXml<JoyDPad>(dpad.next());
            joydpadXml->writeConfig(xml);
            delete joydpadXml;
            joydpadXml = nullptr;
        }

        QList<JoyButton *> buttonsList = m_setJoystick->getButtons().values();
        QListIterator<JoyButton *> button(buttonsList);
        while (button.hasNext())
        {
            JoyButton *buttonCurr = button.next();
            if ((buttonCurr != nullptr) && !buttonCurr->isPartVDPad())
            {
                JoyButtonXml *joyButtonXml = new JoyButtonXml(buttonCurr);
                joyButtonXml->writeConfig(xml);
                delete joyButtonXml;
                joyButtonXml = nullptr;
            }
        }

        xml->writeEndElement();
    }
}
