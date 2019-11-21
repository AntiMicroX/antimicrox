#ifndef INPUTDEVICEXML_H
#define INPUTDEVICEXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;
class InputDevice;
class AntiMicroSettings;

class InputDeviceXml : public QObject
{
    Q_OBJECT
public:
    explicit InputDeviceXml(InputDevice *inputDevice, QObject *parent = nullptr);

public slots:

    virtual void readConfig(QXmlStreamReader *xml); // InputDeviceXml class
    virtual void writeConfig(QXmlStreamWriter *xml); // InputDeviceXml class

private:
    InputDevice* m_inputDevice;

};

#endif // INPUTDEVICEXML_H
