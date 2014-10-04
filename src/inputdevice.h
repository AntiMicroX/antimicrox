#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_platform.h>
#else
#include <SDL/SDL_joystick.h>
typedef Sint32 SDL_JoystickID;
#endif

#include "setjoystick.h"
#include "common.h"

class InputDevice : public QObject
{
    Q_OBJECT
public:
    explicit InputDevice(int deviceIndex, QObject *parent = 0);
    virtual ~InputDevice();

    virtual int getNumberButtons();
    virtual int getNumberAxes();
    virtual int getNumberHats();
    virtual int getNumberSticks();
    virtual int getNumberVDPads();

    int getJoyNumber();
    int getRealJoyNumber();
    int getActiveSetNumber();
    SetJoystick* getActiveSetJoystick();
    SetJoystick* getSetJoystick(int index);
    void removeControlStick(int index);
    bool isActive();
    int getButtonDownCount();

    virtual QString getName() = 0;
    virtual QString getSDLName() = 0;
    // GUID only available on SDL 2.
    virtual QString getGUIDString() = 0;
    virtual QString getStringIdentifier();
    virtual QString getXmlName() = 0;
    virtual void closeSDLDevice() = 0;
#ifdef USE_SDL_2
    virtual SDL_JoystickID getSDLJoystickID() = 0;
    QString getSDLPlatform();
#endif
    virtual bool isGameController();

    void setButtonName(int index, QString tempName);
    void setAxisButtonName(int axisIndex, int buttonIndex, QString tempName);
    void setStickButtonName(int stickIndex, int buttonIndex, QString tempName);
    void setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName);
    void setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName);

    void setAxisName(int axisIndex, QString tempName);
    void setStickName(int stickIndex, QString tempName);
    void setDPadName(int dpadIndex, QString tempName);
    void setVDPadName(int vdpadIndex, QString tempName);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual int getNumberRawButtons() = 0;
    virtual int getNumberRawAxes() = 0;
    virtual int getNumberRawHats() = 0;

    unsigned int getDeviceKeyPressTime();

    void setIndex(int index);
    bool isDeviceEdited();
    void revertProfileEdited();

    void setKeyRepeatStatus(bool enabled);
    void setKeyRepeatDelay(int delay);
    void setKeyRepeatRate(int rate);

    bool isKeyRepeatEnabled();
    int getKeyRepeatDelay();
    int getKeyRepeatRate();

    QString getProfileName();

    static const int NUMBER_JOYSETS;
    static const int DEFAULTKEYPRESSTIME;
    static const unsigned int DEFAULTKEYREPEATDELAY;
    static const unsigned int DEFAULTKEYREPEATRATE;

protected:
    void enableSetConnections(SetJoystick *setstick);

    SDL_Joystick* joyhandle;
    QHash<int, SetJoystick*> joystick_sets;
    int active_set;
    int joyNumber;
    int buttonDownCount;
    SDL_JoystickID joystickID;
    unsigned int keyPressTime;
    bool deviceEdited;

    bool keyRepeatEnabled;
    int keyRepeatDelay;
    int keyRepeatRate;
    QString profileName;

signals:
    void setChangeActivated(int index);
    void setAxisThrottleActivated(int index);
    void clicked(int index);
    void released(int index);

    void rawButtonClick(int index);
    void rawButtonRelease(int index);
    void rawAxisButtonClick(int axis, int buttonindex);
    void rawAxisButtonRelease(int axis, int buttonindex);
    void rawDPadButtonClick(int dpad, int buttonindex);
    void rawDPadButtonRelease(int dpad, int buttonindex);
    void rawAxisActivated(int axis, int value);
    void rawAxisReleased(int axis, int value);
    void profileUpdated();
    void propertyUpdated();
    void profileNameEdited(QString text);
    void mouseCursorMoved(int mouseX, int mouseY, int elapsed);
    void mouseSpringMoved(int coordX, int coordY);

public slots:
    void reset();
    void resetButtonDownCount();
    void setActiveSetNumber(int index);
    void changeSetButtonAssociation(int button_index, int originset, int newset, int mode);
    void changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode);
    void changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode);
    void changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode);
    void changeSetVDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode);
    void setDeviceKeyPressTime(unsigned int newPressTime);
    void profileEdited();
    void setProfileName(QString value);
    void propogateMouseCursorMoved(int mouseX, int mouseY, int elapsed);
    void propogateMouseSpringMoved(int coordX, int coordY);

    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

protected slots:
    void propogateSetChange(int index);
    void propogateSetAxisThrottleChange(int index, int originset);
    void buttonDownEvent(int setindex, int buttonindex);
    void buttonUpEvent(int setindex, int buttonindex);
    virtual void axisActivatedEvent(int setindex, int axisindex, int value);
    virtual void axisReleasedEvent(int setindex, int axisindex, int value);
    virtual void buttonClickEvent(int buttonindex);
    virtual void buttonReleaseEvent(int buttonindex);
    virtual void axisButtonDownEvent(int setindex, int axisindex, int buttonindex);
    virtual void axisButtonUpEvent(int setindex, int axisindex, int buttonindex);
    virtual void dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex);
    virtual void dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex);
    virtual void dpadButtonClickEvent(int buttonindex);
    virtual void dpadButtonReleaseEvent(int buttonindex);
    virtual void stickButtonDownEvent(int setindex, int stickindex, int buttonindex);
    virtual void stickButtonUpEvent(int setindex, int stickindex, int buttonindex);

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

Q_DECLARE_METATYPE(InputDevice*)

#endif // INPUTDEVICE_H
