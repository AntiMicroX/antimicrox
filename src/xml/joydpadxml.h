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


#ifndef JOYDPADXML_H
#define JOYDPADXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

template<class T>
class JoyDPadXml : public QObject
{

public:
    explicit JoyDPadXml(T* joydpad, QObject *parent = nullptr);

    void readConfig(QXmlStreamReader *xml); // JoyDPadXml class
    void writeConfig(QXmlStreamWriter *xml); // JoyDPadXml class
    bool readMainConfig(QXmlStreamReader *xml);

private:

    T* m_joydpad;
};

#include "joydpadxml.cpp"

#endif // JOYDPADXML_H
