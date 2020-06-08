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


#ifndef INPUTDEVICEXML_H
#define INPUTDEVICEXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;
class InputDevice;
class AntiMicroSettings;

class InputDeviceXml : public QObject
{
    Q_OBJECT
public:
    explicit InputDeviceXml(InputDevice *inputDevice, QObject *parent = nullptr);

public slots:

    virtual void readConfig(QXmlStreamReader *xml); // InputDeviceXml class
    virtual void writeConfig(QXmlStreamWriter *xml); // InputDeviceXml class

private:
    InputDevice* m_inputDevice;

};

#endif // INPUTDEVICEXML_H
