#ifndef GAMECONTROLLERDPADXML_H
#define GAMECONTROLLERDPADXML_H

#include "xml/vdpadxml.h"

#include <QObject>

class GameControllerDPad;
class VDPad;
class QXmlStreamReader;


class GameControllerDPadXml : public VDPadXml
{
public:
    GameControllerDPadXml(GameControllerDPad* gameContrDPad, VDPad* vdpad, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml); // GameControllerDPadXml class

private:
    GameControllerDPad* m_gameContrDPad;
};

#endif // GAMECONTROLLERDPADXML_H
