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
