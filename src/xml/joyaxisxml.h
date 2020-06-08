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


#ifndef JOYAXISXML_H
#define JOYAXISXML_H

#include <QObject>
#include <QPointer>

class JoyAxis;
class JoyButtonXml;
class QXmlStreamReader;
class QXmlStreamWriter;

class JoyAxisXml : public QObject
{
    Q_OBJECT

public:
    explicit JoyAxisXml(JoyAxis* axis, QObject *parent = nullptr);
    ~JoyAxisXml();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual bool readMainConfig(QXmlStreamReader *xml);
    virtual bool readButtonConfig(QXmlStreamReader *xml);

private:
    JoyAxis* m_joyAxis;
    QPointer<JoyButtonXml> joyButtonXmlNAxis;
    QPointer<JoyButtonXml> joyButtonXmlPAxis;
};

#endif // JOYAXISXML_H
