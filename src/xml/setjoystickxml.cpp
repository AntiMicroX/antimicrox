#include "setjoystickxml.h"
#include "xml/joybuttonxml.h"
#include "xml/joyaxisxml.h"
#include "xml/joydpadxml.h"
#include <iostream>
#include <memory>

#include "joybutton.h"
#include "joyaxis.h"
#include "joydpad.h"
#include "vdpad.h"
#include "joycontrolstick.h"

#include "setjoystick.h"

#include "messagehandler.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>



SetJoystickXml::SetJoystickXml(SetJoystick* setJoystick, QObject *parent) : QObject(parent), m_setJoystick(setJoystick)
{

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
                joyButtonXml = new JoyButtonXml(button);

                if (button != nullptr) joyButtonXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = m_setJoystick->getJoyAxis(index - 1);
                joyAxisXml = new JoyAxisXml(axis);

                if (axis != nullptr) joyAxisXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = m_setJoystick->getJoyDPad(index - 1);
                JoyDPadXml<JoyDPad>* joydpadXml = new JoyDPadXml<JoyDPad>(dpad);

                if (dpad != nullptr) joydpadXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = m_setJoystick->getJoyStick(stickIndex);

                    if (stick != nullptr) stick->readConfig(xml);
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
                JoyDPadXml<VDPad>* joydpadXml = new JoyDPadXml<VDPad>(vdpad);

                if (vdpad != nullptr) joydpadXml->readConfig(xml);
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
        xml->writeAttribute("index", QString::number(m_setJoystick->getIndex() +1));

        if (!m_setJoystick->getName().isEmpty())
            xml->writeTextElement("name", m_setJoystick->getName());


        QList<JoyControlStick*> sticksList = m_setJoystick->getSticks().values();
        QListIterator<JoyControlStick*> i(sticksList);
        while (i.hasNext())
            i.next()->writeConfig(xml);


        QList<VDPad*> vdpadsList = m_setJoystick->getVdpads().values();
        QListIterator<VDPad*> vdpad(vdpadsList);
        while (vdpad.hasNext())
        {
            JoyDPadXml<VDPad>* joydpadXml = new JoyDPadXml<VDPad>(vdpad.next());
            joydpadXml->writeConfig(xml);
        }


        QList<JoyAxis*> axesList = m_setJoystick->getAxes()->values();
        QListIterator<JoyAxis*> axis(axesList);
        while (axis.hasNext())
        {
            JoyAxis* axisCur = axis.next();
            JoyAxisXml* joyAxisXml = new JoyAxisXml(axisCur);

            if (!axisCur->isPartControlStick() && axisCur->hasControlOfButtons())
            {
                joyAxisXml->writeConfig(xml);
            }
        }


        QList<JoyDPad*> dpadsList = m_setJoystick->getHats().values();
        QListIterator<JoyDPad*> dpad(dpadsList);
        while (dpad.hasNext())
        {
            JoyDPadXml<JoyDPad>* joydpadXml = new JoyDPadXml<JoyDPad>(dpad.next());
            joydpadXml->writeConfig(xml);
        }


        QList<JoyButton*> buttonsList = m_setJoystick->getButtons().values();
        QListIterator<JoyButton*> button(buttonsList);
        while (button.hasNext())
        {
            JoyButton* buttonCurr = button.next();
            if ((buttonCurr != nullptr) && !buttonCurr->isPartVDPad())
            {
                JoyButtonXml* joyButtonXml = new JoyButtonXml(buttonCurr);
                joyButtonXml->writeConfig(xml);
            }
        }

        xml->writeEndElement();
    }
}
