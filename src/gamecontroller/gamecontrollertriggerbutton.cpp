/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gamecontrollertriggerbutton.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "setjoystick.h"
#include "joyaxis.h"
#include "inputdevice.h"
#include "xml/joybuttonxml.h"

#include <QXmlStreamReader>
#include <QDebug>


GameControllerTriggerButton::GameControllerTriggerButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxisButton(axis, index, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


QString GameControllerTriggerButton::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameControllerTriggerButton::xmlName;
}


void GameControllerTriggerButton::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


    if (xml->isStartElement() && (xml->name() == GlobalVariables::JoyAxisButton::xmlName))
    {
        disconnect(this, &GameControllerTriggerButton::slotsChanged, m_parentSet->getInputDevice(), &InputDevice::profileEdited);

        xml->readNextStartElement();


        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != GlobalVariables::JoyAxisButton::xmlName)))
        {
            JoyButtonXml* joyButtonXml = new JoyButtonXml(this);
            bool found = joyButtonXml->readButtonConfig(xml);

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        connect(this, &GameControllerTriggerButton::slotsChanged, m_parentSet->getInputDevice(), &InputDevice::profileEdited);
    }
}
