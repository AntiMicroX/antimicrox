#ifndef SETJOYSTICK_H
#define SETJOYSTICK_H

#include <QObject>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <SDL/SDL_joystick.h>

#include "joyaxis.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joybutton.h"
#include "vdpad.h"

class SetJoystick : public QObject
{
    Q_OBJECT
public:
    explicit SetJoystick(SDL_Joystick *joyhandle, int index, QObject *parent=0);
    ~SetJoystick();

    SDL_Joystick* getSDLHandle ();
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
    void refreshButtons ();
    void refreshAxes();
    void refreshHats();
    void release();
    void addControlStick(int index, JoyControlStick *stick);
    void removeControlStick(int index);
    void addVDPad(int index, VDPad *vdpad);
    void removeVDPad(int index);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

protected:
    bool isSetEmpty();
    void deleteButtons();
    void deleteAxes();
    void deleteHats();
    void deleteSticks();
    void deleteVDpads();

    QHash<int, JoyButton*> buttons;
    QHash<int, JoyAxis*> axes;
    QHash<int, JoyDPad*> hats;
    QHash<int, JoyControlStick*> sticks;
    QHash<int, VDPad*> vdpads;

    int index;
    SDL_Joystick* joyhandle;

signals:
    void setChangeActivated(int index);
    void setAssignmentButtonChanged(int button, int originset, int newset, int mode);
    void setAssignmentAxisChanged(int button, int axis, int originset, int newset, int mode);
    void setAssignmentStickChanged(int button, int stick, int originset, int newset, int mode);
    void setAssignmentDPadChanged(int button, int dpad, int originset, int newset, int mode);
    void setAssignmentAxisThrottleChanged(int axis, int originset);

    
public slots:
    void reset();
    void propogateSetChange(int index);
    void propogateSetButtonAssociation(int button, int newset, int mode);
    void propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode);
    void propogateSetStickButtonAssociation(int button, int stick, int newset, int mode);
    void propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode);


protected slots:
    void propogateSetAxisThrottleSetting(int index);

};

#endif // SETJOYSTICK_H
