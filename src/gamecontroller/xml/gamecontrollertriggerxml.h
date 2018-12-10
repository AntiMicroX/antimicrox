#ifndef GAMECONTROLLERTRIGGERXML_H
#define GAMECONTROLLERTRIGGERXML_H

#include "xml/joyaxisxml.h"

#include <QObject>
#include <QPointer>

class GameControllerTrigger;
class GameContrTriggBtnXml;


class GameControllerTriggerXml : public JoyAxisXml
{

    public:
        GameControllerTriggerXml(GameControllerTrigger* gameContrTrigger, JoyAxis* joyAxis, QObject *parent = 0);

        void readJoystickConfig(QXmlStreamReader *xml);
        virtual void writeConfig(QXmlStreamWriter *xml);

    private:
        GameControllerTrigger* m_gameContrTrigger;
        QPointer<GameContrTriggBtnXml> m_gameContrTriggBtnXml;
        QPointer<JoyButtonXml> m_joyButtonXmlNAxis;
        QPointer<JoyButtonXml> m_joyButtonXmlPAxis;
};

#endif // GAMECONTROLLERTRIGGERXML_H
