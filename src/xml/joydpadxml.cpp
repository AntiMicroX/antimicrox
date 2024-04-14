
#include "joydpadxml.h"
#include "globalvariables.h"

#include "gamecontroller/gamecontrollerdpad.h"
#include "joydpad.h"
#include "vdpad.h"
#include "xml/joybuttonxml.h"

#include <QDebug>
#include <QHashIterator>
#include <QPointer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

template <class T>
JoyDPadXml<T>::JoyDPadXml(T *joydpad, QObject *parent)
    : QObject(parent)
{
    m_joydpad = joydpad;
}

template <class T> void JoyDPadXml<T>::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == m_joydpad->getXmlName()))
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != m_joydpad->getXmlName())))
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

template <class T> void JoyDPadXml<T>::writeConfig(QXmlStreamWriter *xml)
{
    if (!m_joydpad->isDefault())
    {
        xml->writeStartElement(m_joydpad->getXmlName());
        xml->writeAttribute("index", QString::number(m_joydpad->getRealJoyNumber()));

        if (m_joydpad->getJoyMode() == JoyDPad::EightWayMode)
        {
            xml->writeTextElement("mode", "eight-way");
        } else if (m_joydpad->getJoyMode() == JoyDPad::FourWayCardinal)
        {
            xml->writeTextElement("mode", "four-way");
        } else if (m_joydpad->getJoyMode() == JoyDPad::FourWayDiagonal)
        {
            xml->writeTextElement("mode", "diagonal");
        }

        if (m_joydpad->getDPadDelay() > GlobalVariables::JoyDPad::DEFAULTDPADDELAY)
        {
            xml->writeTextElement("dpadDelay", QString::number(m_joydpad->getDPadDelay()));
        }

        QHashIterator<int, JoyDPadButton *> iter(*m_joydpad->getJoyButtons());
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            JoyButtonXml *joyBtnXml = new JoyButtonXml(button);
            joyBtnXml->writeConfig(xml);
        }

        xml->writeEndElement();
    }
}

template <class T> bool JoyDPadXml<T>::readMainConfig(QXmlStreamReader *xml)
{
    bool found = false;

    if ((xml->name().toString() == "dpadbutton") && xml->isStartElement())
    {
        found = true;
        int index_local = xml->attributes().value("index").toString().toInt();
        JoyDPadButton *button = m_joydpad->getJoyButton(index_local);

        if (button != nullptr)
        {
            JoyButtonXml *joyBtnXml = new JoyButtonXml(button);
            joyBtnXml->readConfig(xml);
        } else
        {
            xml->skipCurrentElement();
        }
    } else if ((xml->name().toString() == "mode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "eight-way")
        {
            m_joydpad->setJoyMode(JoyDPad::EightWayMode);
        } else if (temptext == "four-way")
        {
            m_joydpad->setJoyMode(JoyDPad::FourWayCardinal);
        } else if (temptext == "diagonal")
        {
            m_joydpad->setJoyMode(JoyDPad::FourWayDiagonal);
        }
    } else if ((xml->name().toString() == "dpadDelay") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joydpad->setDPadDelay(tempchoice);
    }

    return found;
}

template class JoyDPadXml<JoyDPad>;
template class JoyDPadXml<VDPad>;
template class JoyDPadXml<GameControllerDPad>;
