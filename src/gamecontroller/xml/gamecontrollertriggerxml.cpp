#include "gamecontrollertriggerxml.h"
#include "gamecontroller/gamecontrollertrigger.h"
#include "gamecontroller/gamecontrollertriggerbutton.h"
#include "gamecontroller/xml/gamecontrtriggbtnxml.h"

#include "xml/joyaxisxml.h"
#include "xml/joybuttonxml.h"

#include "messagehandler.h"

#include <SDL2/SDL_gamecontroller.h>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>


GameControllerTriggerXml::GameControllerTriggerXml(GameControllerTrigger* gameContrTrigger, JoyAxis* joyAxis, QObject *parent) : JoyAxisXml(joyAxis, parent)
{
    m_gameContrTrigger = gameContrTrigger;
    m_joyButtonXmlNAxis = new JoyButtonXml(gameContrTrigger->getNAxisButton());
    m_joyButtonXmlPAxis = new JoyButtonXml(gameContrTrigger->getPAxisButton());
}

void GameControllerTriggerXml::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == GlobalVariables::JoyAxis::xmlName))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != GlobalVariables::JoyAxis::xmlName)))
        {
            bool found = readMainConfig(xml);

            if (!found && (xml->name() == GlobalVariables::JoyAxisButton::xmlName) && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "Index for axis in readJoystickConfig is: " << index;
                #endif

                switch (index) {

                    case 1:
                        found = true;

                        if (!m_gameContrTriggBtnXml.isNull()) m_gameContrTriggBtnXml.clear();
                        m_gameContrTriggBtnXml = new GameContrTriggBtnXml(qobject_cast<GameControllerTriggerButton*>(m_gameContrTrigger->getNAxisButton()), m_gameContrTrigger->getNAxisButton());
                        m_gameContrTriggBtnXml->readJoystickConfig(xml);
                    break;

                    case 2:
                        found = true;

                        if (!m_gameContrTriggBtnXml.isNull()) m_gameContrTriggBtnXml.clear();

                        if (!m_gameContrTriggBtnXml.isNull()) m_gameContrTriggBtnXml.clear();
                        m_gameContrTriggBtnXml = new GameContrTriggBtnXml(qobject_cast<GameControllerTriggerButton*>(m_gameContrTrigger->getPAxisButton()), m_gameContrTrigger->getPAxisButton());
                        m_gameContrTriggBtnXml->readJoystickConfig(xml);
                    break;
                }
            }

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }

    if (m_gameContrTrigger->getThrottle() != static_cast<int>(JoyAxis::PositiveHalfThrottle))
    {
        m_gameContrTrigger->setThrottle(static_cast<int>(JoyAxis::PositiveHalfThrottle));
        m_gameContrTrigger->setCurrentRawValue(m_gameContrTrigger->getCurrentThrottledDeadValue());
        m_gameContrTrigger->updateCurrentThrottledValue(m_gameContrTrigger->calculateThrottledValue(m_gameContrTrigger->getCurrentRawValue()));
    }
}

void GameControllerTriggerXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool currentlyDefault = m_gameContrTrigger->isDefault();

    xml->writeStartElement(m_gameContrTrigger->getXmlName());
    xml->writeAttribute("index", QString::number((m_gameContrTrigger->getRealJoyIndex()) - SDL_CONTROLLER_AXIS_TRIGGERLEFT));

    if (!currentlyDefault && (m_gameContrTrigger->getDeadZone() != GlobalVariables::GameControllerTrigger::AXISDEADZONE))
    {
        xml->writeTextElement("deadZone", QString::number(m_gameContrTrigger->getDeadZone()));
    }

    if (!currentlyDefault && (m_gameContrTrigger->getMaxZoneValue() != GlobalVariables::GameControllerTrigger::AXISMAXZONE))
    {
        xml->writeTextElement("maxZone", QString::number(m_gameContrTrigger->getMaxZoneValue()));
    }

    xml->writeStartElement("throttle");

        switch(m_gameContrTrigger->getThrottle())
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

    if (!currentlyDefault)
    {
        m_joyButtonXmlNAxis.data()->writeConfig(xml);
        m_joyButtonXmlPAxis.data()->writeConfig(xml);
    }

    xml->writeEndElement();
}
