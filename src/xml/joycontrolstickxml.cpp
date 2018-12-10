#include "joycontrolstickxml.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"
#include "xml/joybuttonxml.h"

#include "messagehandler.h"
#include "QDebug"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

JoyControlStickXml::JoyControlStickXml(JoyControlStick* stick, QObject *parent) : QObject(parent)
{
    m_stick = stick;
}

/**
 * @brief Take a XML stream and set the stick and direction button properties
 *     according to the values contained within the stream.
 * @param QXmlStreamReader instance that will be used to read property values.
 */
void JoyControlStickXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "stick"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "stick")))
        {
            if ((xml->name() == "deadZone") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                m_stick->setDeadZone(tempchoice);
            }
            else if ((xml->name() == "maxZone") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                m_stick->setMaxZone(tempchoice);
            }
            else if ((xml->name() == "calibrated") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                bool tempchoice = (temptext == "true") ? true : false;
                m_stick->setCalibrationFlag(tempchoice);
            }
            else if ((xml->name() == "summary") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                m_stick->setCalibrationSummary(temptext);
            }
            else if ((xml->name() == "diagonalRange") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                m_stick->setDiagonalRange(tempchoice);
            }
            else if ((xml->name() == "mode") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (temptext == "eight-way")
                {
                    m_stick->setJoyMode(JoyControlStick::EightWayMode);
                }
                else if (temptext == "four-way")
                {
                    m_stick->setJoyMode(JoyControlStick::FourWayCardinal);
                }
                else if (temptext == "diagonal")
                {
                    m_stick->setJoyMode(JoyControlStick::FourWayDiagonal);
                }
            }
            else if ((xml->name() == "squareStick") && xml->isStartElement())
            {
                int tempchoice = xml->readElementText().toInt();

                if ((tempchoice > 0) && (tempchoice <= 100))
                    m_stick->setCircleAdjust(tempchoice / 100.0);
            }
            else if ((xml->name() == GlobalVariables::JoyControlStickButton::xmlName) && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyControlStickButton *button = m_stick->getButtons()->value(static_cast<JoyStickDirections>(index));
                JoyButtonXml* joyButtonXml = new JoyButtonXml(button);

                if (button != nullptr) joyButtonXml->readConfig(xml);
                else xml->skipCurrentElement();
            }
            else if ((xml->name() == GlobalVariables::JoyControlStickModifierButton::xmlName) && xml->isStartElement())
            {
                JoyButtonXml* joyButtonXml = new JoyButtonXml(m_stick->getModifierButton());
                joyButtonXml->readConfig(xml);
            }
            else if ((xml->name() == "stickDelay") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                m_stick->setStickDelay(tempchoice);
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

/**
 * @brief Write the status of the properties of a stick and direction buttons
 *     to an XML stream.
 * @param QXmlStreamWriter instance that will be used to write a profile.
 */
void JoyControlStickXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!m_stick->isDefault())
    {
        xml->writeStartElement("stick");
        xml->writeAttribute("index", QString::number(m_stick->getRealJoyIndex()));

        if (m_stick->getDeadZone() != GlobalVariables::JoyControlStick::DEFAULTDEADZONE)
            xml->writeTextElement("deadZone", QString::number(m_stick->getDeadZone()));

        if (m_stick->getMaxZone() != GlobalVariables::JoyControlStick::DEFAULTMAXZONE)
            xml->writeTextElement("maxZone", QString::number(m_stick->getMaxZone()));

        xml->writeTextElement("calibrated", (m_stick->wasCalibrated() ? "true" : "false"));
        xml->writeTextElement("summary", (m_stick->getCalibrationSummary().isEmpty() ? "" : m_stick->getCalibrationSummary()));


        if ((m_stick->getJoyMode() == JoyControlStick::StandardMode || m_stick->getJoyMode() == JoyControlStick::EightWayMode) && (m_stick->getDiagonalRange() != GlobalVariables::JoyControlStick::DEFAULTDIAGONALRANGE))
        {
            xml->writeTextElement("diagonalRange", QString::number(m_stick->getDiagonalRange()));
        }

        switch(m_stick->getJoyMode())
        {
            case JoyControlStick::EightWayMode:
                xml->writeTextElement("mode", "eight-way");
            break;

            case JoyControlStick::FourWayCardinal:
                xml->writeTextElement("mode", "four-way");
            break;

            case JoyControlStick::FourWayDiagonal:
                xml->writeTextElement("mode", "diagonal");
            break;
        }

        if (m_stick->getCircleAdjust() > GlobalVariables::JoyControlStick::DEFAULTCIRCLE)
            xml->writeTextElement("squareStick", QString::number(m_stick->getCircleAdjust() * 100));

        if (m_stick->getStickDelay() > GlobalVariables::JoyControlStick::DEFAULTSTICKDELAY)
            xml->writeTextElement("stickDelay", QString::number(m_stick->getStickDelay()));

        QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(*m_stick->getButtons());

        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButtonXml* joyButtonXml = new JoyButtonXml(button);
            joyButtonXml->writeConfig(xml);
        }

        if (!m_stick->getModifierButton()->isDefault())
        {
            JoyButtonXml* joyButtonXmlModif = new JoyButtonXml(m_stick->getModifierButton());
            joyButtonXmlModif->writeConfig(xml);
        }

        xml->writeEndElement();
    }
}
