#ifndef SETJOYSTICK_H
#define SETJOYSTICK_H

#include <QObject>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#else
#include <SDL/SDL_joystick.h>
#endif

#include "joyaxis.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joybutton.h"
#include "vdpad.h"

class InputDevice;

class SetJoystick : public QObject
{
    Q_OBJECT
public:
    explicit SetJoystick(InputDevice *device, int index, QObject *parent=0);
    explicit SetJoystick(InputDevice *device, int index, bool runreset, QObject *parent=0);
    ~SetJoystick();

    JoyAxis* getJoyAxis(int index);
    JoyButton* getJoyButton(int index);
    JoyDPad* getJoyDPad(int index);
    JoyControlStick* getJoyStick(int index);
    VDPad *getVDPad(int index);

    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    int getNumberSticks();
    int getNumberVDPads();

    int getIndex();
    virtual void refreshButtons ();
    virtual void refreshAxes();
    virtual void refreshHats();
    void release();
    void addControlStick(int index, JoyControlStick *stick);
    void removeControlStick(int index);
    void addVDPad(int index, VDPad *vdpad);
    void removeVDPad(int index);
    void setIgnoreEventState(bool ignore);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

protected:
    bool isSetEmpty();
    void deleteButtons();
    void deleteAxes();
    void deleteHats();
    void deleteSticks();
    void deleteVDpads();

    void enableButtonConnections(JoyButton *button);
    void enableAxisConnections(JoyAxis *axis);
    void enableHatConnections(JoyDPad *dpad);

    QHash<int, JoyButton*> buttons;
    QHash<int, JoyAxis*> axes;
    QHash<int, JoyDPad*> hats;
    QHash<int, JoyControlStick*> sticks;
    QHash<int, VDPad*> vdpads;

    int index;
    //SDL_Joystick* joyhandle;
    InputDevice *device;

signals:
    void setChangeActivated(int index);
    void setAssignmentButtonChanged(int button, int originset, int newset, int mode);
    void setAssignmentAxisChanged(int button, int axis, int originset, int newset, int mode);
    void setAssignmentStickChanged(int button, int stick, int originset, int newset, int mode);
    void setAssignmentDPadChanged(int button, int dpad, int originset, int newset, int mode);
    void setAssignmentAxisThrottleChanged(int axis, int originset);
    void setButtonClick(int index, int button);
    void setButtonRelease(int index, int button);
    void setAxisButtonClick(int setindex, int axis, int button);
    void setAxisButtonRelease(int setindex, int axis, int button);
    void setAxisActivated(int setindex, int axis, int value);
    void setStickButtonClick(int setindex, int stick, int button);
    void setStickButtonRelease(int setindex, int stick, int button);
    void setDPadButtonClick(int setindex, int dpad, int button);
    void setDPadButtonRelease(int setindex, int dpad, int button);

    void setButtonNameChange(int index);
    void setAxisButtonNameChange(int axisIndex, int buttonIndex);
    void setStickButtonNameChange(int stickIndex, int buttonIndex);
    void setDPadButtonNameChange(int dpadIndex, int buttonIndex);
    void setVDPadButtonNameChange(int vdpadIndex, int buttonIndex);

    void setAxisNameChange(int axisIndex);
    void setStickNameChange(int stickIndex);
    void setDPadNameChange(int dpadIndex);
    void setVDPadNameChange(int vdpadIndex);
    
public slots:
    virtual void reset();
    void propogateSetChange(int index);
    void propogateSetButtonAssociation(int button, int newset, int mode);
    void propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode);
    void propogateSetStickButtonAssociation(int button, int stick, int newset, int mode);
    void propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode);

protected slots:
    void propogateSetAxisThrottleSetting(int index);
    void propogateSetButtonClick(int button);
    void propogateSetButtonRelease(int button);
    void propogateSetAxisButtonClick(int button);
    void propogateSetAxisButtonRelease(int button);
    void propogateSetStickButtonClick(int button);
    void propogateSetStickButtonRelease(int button);
    void propogateSetDPadButtonClick(int button);
    void propogateSetDPadButtonRelease(int button);
    void propogateSetAxisActivated(int value);

    void propogateSetButtonNameChange();
    void propogateSetAxisButtonNameChange();
    void propogateSetStickButtonNameChange();
    void propogateSetDPadButtonNameChange();
    void propogateSetVDPadButtonNameChange();

    void propogateSetAxisNameChange();
    void propogateSetStickNameChange();
    void propogateSetDPadNameChange();
    void propogateSetVDPadNameChange();
};

#endif // SETJOYSTICK_H
