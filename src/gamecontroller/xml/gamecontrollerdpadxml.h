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


#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include "xml/joydpadxml.h"

class GameControllerDPad;
class QXmlStreamReader;


class GameControllerDPadXml : public JoyDPadXml<VDPad>
{
    Q_OBJECT

public:
    explicit GameControllerDPadXml(GameControllerDPad* gameContrDpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml);

private:
    JoyDPadXml<GameControllerDPad>* dpadXml;
};

#endif // GAMECONTROLLERDPADXML_H
