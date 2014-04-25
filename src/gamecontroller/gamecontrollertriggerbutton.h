#ifndef GAMECONTROLLERBUTTON_H
#define GAMECONTROLLERBUTTON_H

#include <QObject>
#include <QXmlStreamReader>

#include <joyaxisbutton.h>

class GameControllerTriggerButton : public JoyAxisButton
{
    Q_OBJECT
public:
    explicit GameControllerTriggerButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent = 0);

    virtual QString getXmlName();
    void readJoystickConfig(QXmlStreamReader *xml);

    static const QString xmlName;

signals:

public slots:

};

#endif // GAMECONTROLLERBUTTON_H
