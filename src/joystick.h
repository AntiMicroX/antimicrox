#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <SDL/SDL_joystick.h>

#include "joyaxis.h"
#include "joydpad.h"
#include "joybutton.h"
#include "setjoystick.h"
#include "common.h"

class Joystick : public QObject
{
    Q_OBJECT
public:
    explicit Joystick(SDL_Joystick *joyhandle, QObject *parent=0);
    ~Joystick();

    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    int getNumberSticks();
    int getNumberVDPads();
    SDL_Joystick* getSDLHandle ();
    int getJoyNumber ();
    int getRealJoyNumber ();
    QString getName();
    int getActiveSetNumber();
    SetJoystick* getActiveSetJoystick();
    SetJoystick* getSetJoystick(int index);
    void removeControlStick(int index);
    bool isActive();
    QString getSDLName();

    void setButtonName(int index, QString tempName);
    void setAxisButtonName(int axisIndex, int buttonIndex, QString tempName);
    void setStickButtonName(int stickIndex, int buttonIndex, QString tempName);
    void setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName);
    void setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName);

    void setAxisName(int axisIndex, QString tempName);
    void setStickName(int stickIndex, QString tempName);
    void setDPadName(int dpadIndex, QString tempName);
    void setVDPadName(int vdpadIndex, QString tempName);

    void resetButtonDownCount();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int NUMBER_JOYSETS;

protected:
    SDL_Joystick* joyhandle;
    QHash<int, SetJoystick*> joystick_sets;
    int active_set;
    int joyNumber;
    int buttonDownCount;

signals:
    void setChangeActivated(int index);
    void setAxisThrottleActivated(int index);
    void clicked(int index);
    void released(int index);

public slots:
    void reset();
    void setActiveSetNumber(int index);
    void changeSetButtonAssociation(int button_index, int originset, int newset, int mode);
    void changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode);
    void changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode);
    void changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode);


private slots:
    void propogateSetChange(int index);
    void propogateSetAxisThrottleChange(int index, int originset);
    void buttonDownEvent(int setindex, int buttonindex);
    void buttonUpEvent(int setindex, int buttonindex);
    void axisButtonDownEvent(int setindex, int axisindex, int buttonindex);
    void axisButtonUpEvent(int setindex, int axisindex, int buttonindex);
    void dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex);
    void dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex);
    void stickButtonDownEvent(int setindex, int stickindex, int buttonindex);
    void stickButtonUpEvent(int setindex, int stickindex, int buttonindex);

    void updateSetButtonNames(int index);
    void updateSetAxisButtonNames(int axisIndex, int buttonIndex);
    void updateSetStickButtonNames(int stickIndex, int buttonIndex);
    void updateSetDPadButtonNames(int dpadIndex, int buttonIndex);
    void updateSetVDPadButtonNames(int vdpadIndex, int buttonIndex);

    void updateSetAxisNames(int axisIndex);
    void updateSetStickNames(int stickIndex);
    void updateSetDPadNames(int dpadIndex);
    void updateSetVDPadNames(int vdpadIndex);
};

Q_DECLARE_METATYPE(Joystick*)

#endif // JOYSTICK_H
