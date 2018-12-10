#include "setjoystickxml.h"
#include "setjoystick.h"
#include "vdpad.h"
#include "xml/joydpadxml.h"
#include "xml/joyaxisxml.h"
#include "xml/joybuttonxml.h"
#include "xml/joycontrolstickxml.h"

#include "messagehandler.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


SetJoystickXml::SetJoystickXml(SetJoystick* setJoystick, QObject* parent) : QObject(parent)
{
    m_setJoystick = setJoystick;
}


void SetJoystickXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "set"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "set")))
        {
            if ((xml->name() == "button") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = m_setJoystick->getJoyButton(index - 1);

                if(!joyButtonXml.isNull()) joyButtonXml.clear();
                joyButtonXml = new JoyButtonXml(button);

                if (button != nullptr && !joyButtonXml.isNull()) joyButtonXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = m_setJoystick->getJoyAxis(index - 1);

                if (!joyAxisXml.isNull()) joyAxisXml.clear();
                joyAxisXml = new JoyAxisXml(axis);

                if (axis != nullptr && !joyAxisXml.isNull()) joyAxisXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = m_setJoystick->getJoyDPad(index - 1);

                if (!joydpadXml.isNull()) joydpadXml.clear();
                joydpadXml = new JoyDPadXml(dpad);

                if (dpad != nullptr && !joydpadXml.isNull()) joydpadXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = m_setJoystick->getJoyStick(stickIndex);

                    if (!joyControlStickXml.isNull()) joyControlStickXml.clear();
                    joyControlStickXml = new JoyControlStickXml(stick);

                    if (stick != nullptr && !joyControlStickXml.isNull()) joyControlStickXml->readConfig(xml);
                    else xml->skipCurrentElement();
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "vdpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = m_setJoystick->getVDPad(index - 1);

                if (!joydpadXml.isNull()) joydpadXml.clear();
                joydpadXml = new JoyDPadXml(vdpad);

                if (vdpad != nullptr && !joydpadXml.isNull()) joydpadXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (!temptext.isEmpty()) m_setJoystick->setName(temptext);
            }
            else
            {
                xml->skipCurrentElement(); // If none of the above, skip the element
            }

            xml->readNextStartElement();
        }
    }
}

void SetJoystickXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!m_setJoystick->isSetEmpty())
    {
        xml->writeStartElement("set");
        xml->writeAttribute("index", QString::number(m_setJoystick->getRealIndex()));

        if (!m_setJoystick->getName().isEmpty())
            xml->writeTextElement("name", m_setJoystick->getName());

        for (int i = 0; i < m_setJoystick->getNumberSticks(); i++)
        {
            JoyControlStick *stick = m_setJoystick->getJoyStick(i);

            if (!joyControlStickXml.isNull()) joyControlStickXml.clear();
            joyControlStickXml = new JoyControlStickXml(stick);

            if (stick != nullptr && !joyControlStickXml.isNull()) joyControlStickXml->writeConfig(xml);
        }

        for (int i = 0; i < m_setJoystick->getNumberVDPads(); i++)
        {
            VDPad *vdpad = m_setJoystick->getVDPad(i);

            if (!joydpadXml.isNull()) joydpadXml.clear();
            joydpadXml = new JoyDPadXml(vdpad);

            if (vdpad != nullptr && !joydpadXml.isNull()) joydpadXml->writeConfig(xml);
        }

        for (int i = 0; i < m_setJoystick->getNumberAxes(); i++)
        {
            JoyAxis *axis = m_setJoystick->getJoyAxis(i);

            if (!joyAxisXml.isNull()) joyAxisXml.clear();
            joyAxisXml = new JoyAxisXml(axis);

            if (!axis->isPartControlStick() && axis->hasControlOfButtons()
                    && (axis != nullptr && !joyAxisXml.isNull()))
            {
                joyAxisXml->writeConfig(xml);
            }
        }

        for (int i = 0; i < m_setJoystick->getNumberHats(); i++)
        {
            JoyDPad *dpad = m_setJoystick->getJoyDPad(i);

            if (!joydpadXml.isNull()) joydpadXml.clear();
            joydpadXml = new JoyDPadXml(dpad);

            if (dpad != nullptr && !joydpadXml.isNull()) joydpadXml->writeConfig(xml);
        }

        for (int i = 0 ; i < m_setJoystick->getNumberButtons(); i++)
        {
            JoyButton *button = m_setJoystick->getJoyButton(i);

            if (!joyButtonXml.isNull()) joyButtonXml.clear();
            joyButtonXml = new JoyButtonXml(button);

            if ((button != nullptr && !joyButtonXml.isNull()) && !button->isPartVDPad())
            {
                joyButtonXml->writeConfig(xml);
            }
        }

        xml->writeEndElement();
    }
}
