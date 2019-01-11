#ifndef GAMECONTROLLERSETXML_H
#define GAMECONTROLLERSETXML_H

#include "xml/setjoystickxml.h"
#include <SDL2/SDL_gamecontroller.h>

#include <QObject>
#include <QList>

class GameControllerSet;
class JoyDPadXml;
class QXmlStreamReader;


class GameControllerSetXml : public SetJoystickXml
{

public:
    GameControllerSetXml(GameControllerSet* gameContrSet, QObject* parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml); // GameControllerSetXml
    virtual void readJoystickConfig(QXmlStreamReader *xml,
                            QHash<int, SDL_GameControllerButton> &buttons,
                            QHash<int, SDL_GameControllerAxis> &axes,
                            QList<SDL_GameControllerButtonBind> &hatButtons); // GameControllerSetXml class

private:
    void getElemFromXml(QString elemName, QXmlStreamReader *xml); // GameControllerSetXml class
    void readConfDpad(QXmlStreamReader *xml, QList<SDL_GameControllerButtonBind> &hatButtons, bool vdpadExists, bool dpadExists); // GameControllerSetXml class

    GameControllerSet* m_gameContrSet;
    JoyDPadXml* dpadXml;

};

#endif // GAMECONTROLLERSETXML_H
