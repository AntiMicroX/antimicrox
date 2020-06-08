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


#ifndef GAMECONTROLLERXML_H
#define GAMECONTROLLERXML_H

#include "gamecontroller/gamecontroller.h"
#include "xml/inputdevicexml.h"

class SetJoystick;
class QXmlStreamReader;
class QXmlStreamWriter;
class JoyAxisButton;
class JoyAxis;


class GameControllerXml : public InputDeviceXml
{
    Q_OBJECT

public:
    explicit GameControllerXml(GameController* gameController, QObject *parent = nullptr);

protected:
    void readJoystickConfig(QXmlStreamReader *xml); // GameControllerXml class

public slots:
    virtual void readConfig(QXmlStreamReader *xml) override; // GameControllerXml class
    virtual void writeConfig(QXmlStreamWriter *xml) override; // GameControllerXml class

private:

    GameController* m_gameController;

    void writeXmlForButtons(SetJoystick *tempSet, QXmlStreamWriter *xml); // GameControllerXml class
    void writeXmlForAxes(SetJoystick *tempSet, QXmlStreamWriter *xml); // GameControllerXml class
    void writeXmlAxBtn(JoyAxis *axis, JoyAxisButton *naxisbutton, QXmlStreamWriter *xml); // GameControllerXml class
    void writeXmlForSticks(SetJoystick *tempSet, QXmlStreamWriter *xml); // GameControllerXml class
    void writeXmlForVDpad(QXmlStreamWriter *xml); // GameControllerXml class
    void readXmlNamesShort(QString name, QXmlStreamReader *xml); // GameControllerXml class
    void readXmlNamesMiddle(QString name, QXmlStreamReader *xml); // GameControllerXml class
    void readXmlNamesLong(QString name, QXmlStreamReader *xml); // GameControllerXml class
    void readJoystickConfigXmlLong(QList<SDL_GameControllerButtonBind>& hatButtons, bool& dpadNameExists, bool& vdpadNameExists, QXmlStreamReader *xml); // GameControllerXml class

    inline void assignVariables(QXmlStreamReader *xml, int& index, int& buttonIndex, QString& temp, bool buttonDecreased); // GameControllerXml class
    inline void assignVariablesShort(QXmlStreamReader *xml, int& index, QString& temp); // GameControllerXml class

};

#endif // GAMECONTROLLERXML_H
