#ifndef JOYCONTROLSTICKMODIFIERBUTTON_H
#define JOYCONTROLSTICKMODIFIERBUTTON_H

#include "joybuttontypes/joygradientbutton.h"

class JoyControlStick;

class JoyControlStickModifierButton : public JoyGradientButton
{
    Q_OBJECT
public:
    explicit JoyControlStickModifierButton(JoyControlStick *stick, int originset, SetJoystick *parentSet, QObject *parent = 0);

    //virtual int getRealJoyNumber();
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual double getLastMouseDistanceFromDeadZone();

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyControlStick *getStick();
    virtual void setTurboMode(TurboMode mode);
    virtual bool isPartRealAxis();
    virtual bool isModifierButton();

    virtual double getAccelerationDistance();
    virtual double getLastAccelerationDistance();

    static const QString xmlName;

protected:
    JoyControlStick *stick;

};

#endif // JOYCONTROLSTICKMODIFIERBUTTON_H
