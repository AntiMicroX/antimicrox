#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include "xml/joydpadxml.h"

#include <QObject>

class GameControllerDPad;
class VDPad;

class GameControllerDPadXml : public JoyDPadXml
{
public:
    GameControllerDPadXml(GameControllerDPad* gameContrDPad, VDPad* vdpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml); // GameControllerDPadXml class

private:
    GameControllerDPad* m_gameContrDPad;
};

#endif // GAMECONTROLLERDPADXML_H
