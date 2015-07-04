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

#include "gamecontrollerdpad.h"

const QString GameControllerDPad::xmlName = "dpad";

GameControllerDPad::GameControllerDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                                       int index, int originset, SetJoystick *parentSet, QObject *parent) :
    VDPad(upButton, downButton, leftButton, rightButton, index, originset, parentSet, parent)
{
}

QString GameControllerDPad::getName(bool forceFullFormat, bool displayName)
{
    QString label;

    if (!dpadName.isEmpty() && displayName)
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(dpadName);
    }
    else if (!defaultDPadName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(defaultDPadName);
    }
    else
    {
        label.append(tr("DPad")).append(" ");
        label.append(QString::number(getRealJoyNumber()));
    }

    return label;
}

QString GameControllerDPad::getXmlName()
{
    return this->xmlName;
}

void GameControllerDPad::readJoystickConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == VDPad::xmlName)
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != VDPad::xmlName))
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
