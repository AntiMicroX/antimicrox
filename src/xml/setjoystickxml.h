/* antimicrox Gamepad to KB+M event mapper
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

#ifndef SETJOYSTICKXML_H
#define SETJOYSTICKXML_H

#include <QObject>

class SetJoystick;
class JoyAxisXml;
class JoyButtonXml;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief SetJoystick XML serialization/deserialization helper class
 *  Reads data from the supplied SetJoystick object and writes it to XML or
 *  reads data from an QXmlStreamReader and writes it to the SetJoystick object.
 */
class SetJoystickXml : public QObject
{
    Q_OBJECT

  public:
    explicit SetJoystickXml(SetJoystick *setJoystick, QObject *parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

  private:
    SetJoystick *m_setJoystick;

    // JoyDPadXml* joydpadXml;
    JoyAxisXml *joyAxisXml;
    JoyButtonXml *joyButtonXml;
};

#endif // SETJOYSTICKXML_H
