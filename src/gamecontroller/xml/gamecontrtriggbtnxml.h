#ifndef GAMECONTRTRIGGBTNXML_H
#define GAMECONTRTRIGGBTNXML_H

#include <QPointer>

#include "xml/joybuttonxml.h"

class GameControllerTriggerButton;
class QXmlStreamReader;
class JoyButtonXml;


class GameContrTriggBtnXml : public JoyButtonXml
{

public:
    GameContrTriggBtnXml(GameControllerTriggerButton* gameContrTriggBtn, JoyButton* joyBtn, QObject* parent = nullptr);

    void readJoystickConfig(QXmlStreamReader *xml);

private:
    GameControllerTriggerButton* m_gameContrTriggBtn;
    QPointer<JoyButtonXml> m_joyButtonXml;

};

#endif // GAMECONTRTRIGGBTNXML_H
