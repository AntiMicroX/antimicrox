#pragma once
#ifndef SETJOYSTICKXML_H
#define SETJOYSTICKXML_H

#include <QObject>

class SetJoystick;
class JoyAxisXml;
class JoyButtonXml;
class QXmlStreamReader;
class QXmlStreamWriter;


class SetJoystickXml : public QObject
{
    Q_OBJECT

public:
    explicit SetJoystickXml(SetJoystick* setJoystick, QObject *parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

private:
    SetJoystick* m_setJoystick;

    //JoyDPadXml* joydpadXml;
    JoyAxisXml* joyAxisXml;
    JoyButtonXml* joyButtonXml;

};

#endif // SETJOYSTICKXML_H
