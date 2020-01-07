#pragma once
#ifndef GAMECONTROLLERTRIGGERXML_H
#define GAMECONTROLLERTRIGGERXML_H

#include "xml/joyaxisxml.h"

class GameControllerTrigger;


class GameControllerTriggerXml : public JoyAxisXml
{

    public:
        GameControllerTriggerXml(GameControllerTrigger* gameContrTrigger, QObject *parent = 0);

        void readJoystickConfig(QXmlStreamReader *xml);
        virtual void writeConfig(QXmlStreamWriter *xml);

    private:
        GameControllerTrigger* m_gameContrTrigger;
        JoyButtonXml* joyButtonXmlNAxis;
        JoyButtonXml* joyButtonXmlPAxis;
};

#endif // GAMECONTROLLERTRIGGERXML_H
