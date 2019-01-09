#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include <QObject>

#include "xml/joydpadxml.h"

class GameControllerDPad;
class QXmlStreamReader;


class GameControllerDPadXml : public JoyDPadXml
{
public:
    GameControllerDPadXml(GameControllerDPad* gameContrDpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml);

private:
    GameControllerDPad* m_gameContrDpad;
    JoyDPadXml* dpadXml;
};

#endif // GAMECONTROLLERDPADXML_H
