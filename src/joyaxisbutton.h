#ifndef JOYAXISBUTTON_H
#define JOYAXISBUTTON_H

#include <QString>

#include "joybutton.h"

class JoyAxis;

class JoyAxisButton : public JoyButton
{
    Q_OBJECT
public:
    explicit JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    virtual double getDistanceFromDeadZone();
    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyAxis* getAxis();
    virtual void setVDPad(VDPad *vdpad);

    static const QString xmlName;

protected:
    JoyAxis *axis;

signals:
    void setAssignmentChanged(int current_button, int axis_index, int associated_set, int mode);
    
public slots:
};

#endif // JOYAXISBUTTON_H
