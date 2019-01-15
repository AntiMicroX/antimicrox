#include "gamecontrollersetxml.h"
#include "gamecontroller/gamecontrollerset.h"
#include "gamecontroller/gamecontrollerdpad.h"
#include "gamecontrollerdpadxml.h"
#include "joycontrolstick.h"
#include "xml/joydpadxml.h"
#include "xml/joyaxisxml.h"
#include "gamecontroller/xml/gamecontrollertriggerxml.h"
#include "xml/joybuttonxml.h"

#include "messagehandler.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>


GameControllerSetXml::GameControllerSetXml(GameControllerSet* gameContrSet, QObject* parent) : SetJoystickXml(gameContrSet, parent), m_gameContrSet(gameContrSet)
{

}


template <typename T>
void readConf(T* x, QXmlStreamReader *xml)
{
    if (x != nullptr)
    {
        x->readConfig(xml);
    }
    else
    {
        xml->skipCurrentElement();
    }
}


void GameControllerSetXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "set"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "set"))
        {
            if ((xml->name() == "button") && xml->isStartElement())
            {
                getElemFromXml("button", xml);
            }
            else if ((xml->name() == "trigger") && xml->isStartElement())
            {
                getElemFromXml("trigger", xml);
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                getElemFromXml("stick", xml);
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                getElemFromXml("dpad", xml);
            }
            else if ((xml->name() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (!temptext.isEmpty())
                {
                    m_gameContrSet->setName(temptext);
                }
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}


void GameControllerSetXml::readJoystickConfig(QXmlStreamReader *xml,
                                           QHash<int, SDL_GameControllerButton> &buttons,
                                           QHash<int, SDL_GameControllerAxis> &axes,
                                           QList<SDL_GameControllerButtonBind> &hatButtons)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "set"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "set")))
        {
            bool dpadExists = false;
            bool vdpadExists = false;

            if ((xml->name() == "button") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = nullptr;
                JoyButtonXml* joyButtonXml = nullptr;

                if (buttons.contains(index-1))
                {
                    SDL_GameControllerButton current = buttons.value(index-1);
                    button = m_gameContrSet->getJoyButton(current);
                    joyButtonXml = new JoyButtonXml(button);
                }

                readConf(joyButtonXml, xml);

            }
            else if ((xml->name() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerTrigger *trigger = nullptr;
                GameControllerTriggerXml* gameContrTriggerXml = nullptr;

                if (axes.contains(index-1))
                {
                    SDL_GameControllerAxis current = axes.value(index-1);
                    trigger = qobject_cast<GameControllerTrigger*>(m_gameContrSet->getJoyAxis(static_cast<int>(current)));
                    gameContrTriggerXml = new GameControllerTriggerXml(trigger);
                }

                if (trigger != nullptr)
                {
                    gameContrTriggerXml->readJoystickConfig(xml);
                    delete gameContrTriggerXml;
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                readConfDpad(xml, hatButtons, vdpadExists, dpadExists);
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                getElemFromXml("stick", xml);
            }
            else if ((xml->name() == "vdpad") && xml->isStartElement())
            {
               readConfDpad(xml, hatButtons, vdpadExists, dpadExists);
            }
            else if ((xml->name() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (!temptext.isEmpty())
                {
                    m_gameContrSet->setName(temptext);
                }
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}


void GameControllerSetXml::readConfDpad(QXmlStreamReader *xml, QList<SDL_GameControllerButtonBind> &hatButtons, bool vdpadExists, bool dpadExists)
{
    int index = xml->attributes().value("index").toString().toInt();
    index = index - 1;
    bool found = false;
    QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
    SDL_GameControllerButtonBind current;

    while (iter.hasNext())
    {
        current = iter.next();

        if (current.value.hat.hat == index)
        {
            found = true;
            iter.toBack();
        }
    }

    VDPad *dpad = nullptr;

    if (found)
    {
        dpad = m_gameContrSet->getVDPad(0);
        dpadXml = new JoyDPadXml<VDPad>(dpad);
    }

    if ((dpad != nullptr) && !vdpadExists)
    {
        dpadExists = true;
        dpadXml->readConfig(xml);
    }
    else
    {
        xml->skipCurrentElement();
    }
}



void GameControllerSetXml::getElemFromXml(QString elemName, QXmlStreamReader *xml)
{
    int index = xml->attributes().value("index").toString().toInt();

    if (elemName == "button") {
        JoyButton *button = m_gameContrSet->getJoyButton(index-1);
        JoyButtonXml* joyButtonXml = new JoyButtonXml(button, this);
        readConf(joyButtonXml, xml);
    }
    else if (elemName == "dpad") {
        GameControllerDPad *vdpad = qobject_cast<GameControllerDPad*>(m_gameContrSet->getVDPad(index-1));
        GameControllerDPadXml* gameContrDPadXml = new GameControllerDPadXml(vdpad);
        readConf(gameContrDPadXml, xml);
    }
    else if (elemName == "trigger")
    {
        GameControllerTrigger *axis = nullptr;
        GameControllerTriggerXml* triggerAxisXml = nullptr;

        switch(index-1)
        {
            case 4:
                axis = qobject_cast<GameControllerTrigger*>(m_gameContrSet->getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT));
                triggerAxisXml = new GameControllerTriggerXml(axis, this);
                readConf(triggerAxisXml, xml);
            break;
            case 5:
                axis = qobject_cast<GameControllerTrigger*>(m_gameContrSet->getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
                triggerAxisXml = new GameControllerTriggerXml(axis, this);
                readConf(triggerAxisXml, xml);
            break;
        }
    }
    else if (elemName == "stick") {
        if (index > 0)
        {
            index -= 1;
            JoyControlStick *stick = m_gameContrSet->getJoyStick(index);
            readConf(stick, xml);
        }
        else
        {
            xml->skipCurrentElement();
        }
    }
}
