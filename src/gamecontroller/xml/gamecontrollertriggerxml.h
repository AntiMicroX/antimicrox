#ifndef GAMECONTROLLERTRIGGERXML_H
#define GAMECONTROLLERTRIGGERXML_H

#include "xml/joyaxisxml.h"

#include <QObject>

class GameControllerTrigger;


class GameControllerTriggerXml : public JoyAxisXml
{

    public:
        GameControllerTriggerXml(GameControllerTrigger* gameContrTrigger, JoyAxis* joyAxis, QObject *parent = 0);

        void readJoystickConfig(QXmlStreamReader *xml);
        virtual void writeConfig(QXmlStreamWriter *xml);

    private:
        GameControllerTrigger* m_gameContrTrigger;
};

#endif // GAMECONTROLLERTRIGGERXML_H
