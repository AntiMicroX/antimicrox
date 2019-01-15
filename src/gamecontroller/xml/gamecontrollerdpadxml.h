#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include <QObject>

#include "xml/joydpadxml.h"
#include "vdpad.h"

class GameControllerDPad;
class QXmlStreamReader;


class GameControllerDPadXml : public JoyDPadXml<VDPad>
{
    Q_OBJECT

public:
    GameControllerDPadXml(GameControllerDPad* gameContrDpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml);

private:
    GameControllerDPad* m_gameContrDpad;
    JoyDPadXml<VDPad>* dpadXml;
};

#endif // GAMECONTROLLERDPADXML_H
