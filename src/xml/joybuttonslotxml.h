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

#pragma once
#ifndef JOYBUTTONSLOTXML_H
#define JOYBUTTONSLOTXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;
class JoyButtonSlot;


class JoyButtonSlotXml : public QObject
{
    Q_OBJECT

public:
     JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent = nullptr);

     virtual void readConfig(QXmlStreamReader *xml);
     virtual void writeConfig(QXmlStreamWriter *xml);

private:
     JoyButtonSlot* m_joyBtnSlot;
};

#endif // JOYBUTTONSLOTXML_H
