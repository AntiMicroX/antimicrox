#ifndef INPUTDEVICEXML_H
#define INPUTDEVICEXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;
class InputDevice;
class AntiMicroSettings;
class SetJoystick;

class InputDeviceXml : public QObject
{
    Q_OBJECT
public:
    explicit InputDeviceXml(InputDevice *inputDevice, QObject *parent = nullptr);
    InputDeviceXml(int deviceIndex, AntiMicroSettings *setting, QObject *parent = nullptr);

public slots:

    virtual void readConfig(QXmlStreamReader *xml); // InputDeviceXml class
    virtual void writeConfig(QXmlStreamWriter *xml); // InputDeviceXml class

private:

    static void removeVDPads(SetJoystick* setJoy);

    InputDevice* m_inputDevice;

};

#endif // INPUTDEVICEXML_H
