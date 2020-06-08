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


#ifndef JOYBUTTONSLOTXML_H
#define JOYBUTTONSLOTXML_H

#include <QObject>
#include <QReadWriteLock>

class QXmlStreamReader;
class QXmlStreamWriter;
class JoyButtonSlot;


class JoyButtonSlotXml : public QObject
{
    Q_OBJECT

public:
     explicit JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent = nullptr);

     virtual void readConfig(QXmlStreamReader *xml);
     virtual void writeConfig(QXmlStreamWriter *xml);
     static int timeoutWrite;
     static int timeoutRead;

private:
     void writeEachSlot(QXmlStreamWriter *xml, JoyButtonSlot *joyBtnSlot);
     void readEachSlot(QXmlStreamReader *xml,  JoyButtonSlot* joyBtnSlot, QString &profile, QString &tempStringData, QString &extraStringData);
     void setSlotData(JoyButtonSlot *joyBtnSlot, QString profile, QString tempStringData, QString extraStringData);

     JoyButtonSlot* m_joyBtnSlot;
     QReadWriteLock xmlLock;

};

#endif // JOYBUTTONSLOTXML_H
