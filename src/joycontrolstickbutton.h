#ifndef JOYCONTROLSTICKBUTTON_H
#define JOYCONTROLSTICKBUTTON_H\

#include "joybutton.h"

class JoyControlStick;

class JoyControlStickButton : public JoyButton
{
    Q_OBJECT
public:
    explicit JoyControlStickButton(JoyControlStick *stick, int index, int originset, QObject *parent = 0);

    virtual QString getPartialName();
    virtual double getDistanceFromDeadZone();

protected:
    JoyControlStick *stick;

signals:
    
public slots:
    virtual void mouseEvent();
};

#endif // JOYCONTROLSTICKBUTTON_H
