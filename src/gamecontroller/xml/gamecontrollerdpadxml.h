#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include "xml/joydpadxml.h"

class GameControllerDPad;
class QXmlStreamReader;


class GameControllerDPadXml : public JoyDPadXml<VDPad>
{
    Q_OBJECT

public:
    GameControllerDPadXml(GameControllerDPad* gameContrDpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml);

private:
    JoyDPadXml<GameControllerDPad>* dpadXml;
};

#endif // GAMECONTROLLERDPADXML_H
