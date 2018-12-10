#ifndef SETJOYSTICKXML_H
#define SETJOYSTICKXML_H

#include <QObject>
#include <QPointer>

class SetJoystick;
class JoyDPadXml;
class JoyAxisXml;
class JoyButtonXml;
class JoyControlStickXml;
class VDPadXml;

class QXmlStreamReader;
class QXmlStreamWriter;

class SetJoystickXml : public QObject
{
public:
    SetJoystickXml(SetJoystick* setJoystick, QObject* parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

private:
    SetJoystick* m_setJoystick;

    QPointer<JoyDPadXml> joydpadXml;
    QPointer<JoyAxisXml> joyAxisXml;
    QPointer<JoyButtonXml> joyButtonXml;
    QPointer<JoyControlStickXml> joyControlStickXml;
    QPointer<VDPadXml> vdpadXml;
};

#endif // SETJOYSTICKXML_H
