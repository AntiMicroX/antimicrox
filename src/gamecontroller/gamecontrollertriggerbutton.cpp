#include <setjoystick.h>
#include <inputdevice.h>

#include "gamecontrollertriggerbutton.h"

const QString GameControllerTriggerButton::xmlName = "triggerbutton";

GameControllerTriggerButton::GameControllerTriggerButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxisButton(axis, index, originset, parentSet, parent)
{
}

QString GameControllerTriggerButton::getXmlName()
{
    return this->xmlName;
}

void GameControllerTriggerButton::readJoystickConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == JoyAxisButton::xmlName)
    {
        disconnect(this, SIGNAL(slotsChanged()), parentSet->getInputDevice(), SLOT(profileEdited()));

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != JoyAxisButton::xmlName))
        {
            bool found = readButtonConfig(xml);
            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        connect(this, SIGNAL(slotsChanged()), parentSet->getInputDevice(), SLOT(profileEdited()));
    }
}
