#include "gamecontrtriggbtnxml.h"
#include "gamecontroller/gamecontrollertriggerbutton.h"
#include "xml/joybuttonxml.h"
#include "inputdevice.h"

#include "messagehandler.h"

#include <QDebug>
#include <QXmlStreamReader>


GameContrTriggBtnXml::GameContrTriggBtnXml(GameControllerTriggerButton* gameContrTriggBtn, JoyButton* joyBtn, QObject* parent) : JoyButtonXml(joyBtn, parent)
{
    m_gameContrTriggBtn = gameContrTriggBtn;
}

void GameContrTriggBtnXml::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


    if (xml->isStartElement() && (xml->name() == GlobalVariables::JoyAxisButton::xmlName))
    {
        disconnect(m_gameContrTriggBtn, &GameControllerTriggerButton::slotsChanged, m_gameContrTriggBtn->getParentSet()->getInputDevice(), &InputDevice::profileEdited);

        xml->readNextStartElement();


        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != GlobalVariables::JoyAxisButton::xmlName)))
        {
            bool found = readButtonConfig(xml);

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        connect(m_gameContrTriggBtn, &GameControllerTriggerButton::slotsChanged, m_gameContrTriggBtn->getParentSet()->getInputDevice(), &InputDevice::profileEdited);
    }
}
