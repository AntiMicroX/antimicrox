#include "joydpadxml.h"
#include "joydpad.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "inputdevice.h"
#include "xml/joybuttonxml.h"

#include <QDebug>
#include <QPointer>
#include <QHashIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>



JoyDPadXml::JoyDPadXml(JoyDPad* joydpad, QObject *parent) : QObject(parent)
{
    m_joydpad = joydpad;
}


void JoyDPadXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == m_joydpad->getXmlName()))
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != m_joydpad->getXmlName())))
        {
            bool found = readMainConfig(xml);
            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyDPadXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!m_joydpad->isDefault())
    {
        xml->writeStartElement(m_joydpad->getXmlName());
        xml->writeAttribute("index", QString::number(m_joydpad->getRealJoyNumber()));
        if (m_joydpad->getJoyMode() == JoyDPad::EightWayMode)
        {
            xml->writeTextElement("mode", "eight-way");
        }
        else if (m_joydpad->getJoyMode() == JoyDPad::FourWayCardinal)
        {
            xml->writeTextElement("mode", "four-way");
        }
        else if (m_joydpad->getJoyMode() == JoyDPad::FourWayDiagonal)
        {
            xml->writeTextElement("mode", "diagonal");
        }

        if (m_joydpad->getDPadDelay() > GlobalVariables::JoyDPad::DEFAULTDPADDELAY)
        {
            xml->writeTextElement("dpadDelay", QString::number(m_joydpad->getDPadDelay()));
        }

        QHashIterator<int, JoyDPadButton*> iter(*m_joydpad->getJoyButtons());
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            JoyButtonXml* joyBtnXml = new JoyButtonXml(button);
            joyBtnXml->writeConfig(xml);
        }

        xml->writeEndElement();
    }
}

bool JoyDPadXml::readMainConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    if ((xml->name() == "dpadbutton") && xml->isStartElement())
    {
        found = true;
        int index_local = xml->attributes().value("index").toString().toInt();
        JoyDPadButton* button = m_joydpad->getJoyButton(index_local);
        if (button != nullptr)
        {
            QPointer<JoyButtonXml> joyBtnXml = new JoyButtonXml(button);
            joyBtnXml->readConfig(xml);

            if (!joyBtnXml.isNull()) delete joyBtnXml;
        }
        else
        {
            xml->skipCurrentElement();
        }
    }
    else if ((xml->name() == "mode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "eight-way")
        {
            m_joydpad->setJoyMode(JoyDPad::EightWayMode);
        }
        else if (temptext == "four-way")
        {
            m_joydpad->setJoyMode(JoyDPad::FourWayCardinal);
        }
        else if (temptext == "diagonal")
        {
            m_joydpad->setJoyMode(JoyDPad::FourWayDiagonal);
        }
    }
    else if ((xml->name() == "dpadDelay") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joydpad->setDPadDelay(tempchoice);
    }

    return found;
}
