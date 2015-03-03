#ifndef JOYAXISBUTTON_H
#define JOYAXISBUTTON_H

#include <QString>

#include "joybuttontypes/joygradientbutton.h"

class JoyAxis;

class JoyAxisButton : public JoyGradientButton
{
    Q_OBJECT
public:
    explicit JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent=0);

    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyAxis* getAxis();
    virtual void setVDPad(VDPad *vdpad);
    virtual void setTurboMode(TurboMode mode);
    virtual bool isPartRealAxis();

    virtual double getLastMouseDistanceFromDeadZone();

    static const QString xmlName;

protected:
    JoyAxis *axis;

signals:
    void setAssignmentChanged(int current_button, int axis_index, int associated_set, int mode);

};

#endif // JOYAXISBUTTON_H
