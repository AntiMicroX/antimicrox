/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "gamecontrollerdpadxml.h"

#include "gamecontroller/gamecontrollerdpad.h"

#include <QDebug>
#include <QXmlStreamReader>

GameControllerDPadXml::GameControllerDPadXml(GameControllerDPad *gameContrDpad, QObject *parent)
    : JoyDPadXml<VDPad>(gameContrDpad, parent)
{
    dpadXml = new JoyDPadXml<GameControllerDPad>(gameContrDpad, this);
}

void GameControllerDPadXml::readJoystickConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name() == GlobalVariables::VDPad::xmlName))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != GlobalVariables::VDPad::xmlName)))
        {
            bool found = dpadXml->readMainConfig(xml);

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}
