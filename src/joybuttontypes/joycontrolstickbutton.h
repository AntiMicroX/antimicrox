#ifndef JOYCONTROLSTICKBUTTON_H
#define JOYCONTROLSTICKBUTTON_H\

#include <QString>

#include "joybuttontypes/joygradientbutton.h"
#include "joycontrolstickdirectionstype.h"

class JoyControlStick;

class JoyControlStickButton : public JoyGradientButton
{
    Q_OBJECT
public:
    explicit JoyControlStickButton(JoyControlStick *stick, int index, int originset, SetJoystick *parentSet, QObject *parent = 0);
    explicit JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index, int originset, SetJoystick *parentSet, QObject *parent = 0);

    virtual int getRealJoyNumber();
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();
    QString getDirectionName();
    JoyStickDirectionsType::JoyStickDirections getDirection();
    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    JoyControlStick *getStick();
    virtual void setTurboMode(TurboMode mode);
    virtual bool isPartRealAxis();

    static const QString xmlName;

protected:
    JoyControlStick *stick;

signals:
    void setAssignmentChanged(int current_button, int axis_index, int associated_set, int mode);

protected slots:
    //virtual void turboEvent();
};

#endif // JOYCONTROLSTICKBUTTON_H
