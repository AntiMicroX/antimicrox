/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include "setjoystick.h"
#include "joyaxis.h"
#include "inputdevice.h"

#include <QXmlStreamReader>
#include <QDebug>

const QString GameControllerTriggerButton::xmlName = "triggerbutton";

GameControllerTriggerButton::GameControllerTriggerButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxisButton(axis, index, originset, parentSet, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
}

QString GameControllerTriggerButton::getXmlName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return this->xmlName;
}

void GameControllerTriggerButton::readJoystickConfig(QXmlStreamReader *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (xml->isStartElement() && (xml->name() == JoyAxisButton::xmlName))
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
