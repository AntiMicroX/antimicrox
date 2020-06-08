/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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


#ifndef GAMECONTROLLERTRIGGERXML_H
#define GAMECONTROLLERTRIGGERXML_H

#include "xml/joyaxisxml.h"

class GameControllerTrigger;


class GameControllerTriggerXml : public JoyAxisXml
{

    public:
        explicit GameControllerTriggerXml(GameControllerTrigger* gameContrTrigger, QObject *parent = 0);

        void readJoystickConfig(QXmlStreamReader *xml);
        virtual void writeConfig(QXmlStreamWriter *xml);

    private:
        GameControllerTrigger* m_gameContrTrigger;
        JoyButtonXml* joyButtonXmlNAxis;
        JoyButtonXml* joyButtonXmlPAxis;
};

#endif // GAMECONTROLLERTRIGGERXML_H
