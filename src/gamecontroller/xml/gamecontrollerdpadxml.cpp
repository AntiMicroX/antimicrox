#include "gamecontrollerdpadxml.h"
#include "gamecontroller/gamecontrollerdpad.h"
#include "vdpad.h"

#include "messagehandler.h"

#include <QXmlStreamReader>
#include <QDebug>

GameControllerDPadXml::GameControllerDPadXml(GameControllerDPad* gameContrDPad, VDPad* vdpad, QObject* parent) : VDPadXml(vdpad, parent)
{
    m_gameContrDPad = gameContrDPad;
}


void GameControllerDPadXml::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == GlobalVariables::VDPad::xmlName))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != GlobalVariables::VDPad::xmlName)))
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
