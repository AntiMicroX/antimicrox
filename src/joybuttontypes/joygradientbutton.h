#ifndef JOYGRADIENTBUTTON_H
#define JOYGRADIENTBUTTON_H

#include "joybutton.h"

class JoyGradientButton : public JoyButton
{
    Q_OBJECT
public:
    explicit JoyGradientButton(int index, int originset, SetJoystick *parentSet, QObject *parent=0);

signals:

protected slots:
    virtual void turboEvent();

};

#endif // JOYGRADIENTBUTTON_H
