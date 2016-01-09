/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QObject>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QRegExp>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_platform.h>
#else
#include <SDL/SDL_joystick.h>
typedef Sint32 SDL_JoystickID;
#endif

#include "setjoystick.h"
#include "common.h"
#include "antimicrosettings.h"

class InputDevice : public QObject
{
    Q_OBJECT
public:
    explicit InputDevice(int deviceIndex, AntiMicroSettings *settings, QObject *parent = 0);
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
    virtual QString getRawGUIDString();

    virtual QString getStringIdentifier();
    virtual QString getXmlName() = 0;
    virtual void closeSDLDevice() = 0;
#ifdef USE_SDL_2
    virtual SDL_JoystickID getSDLJoystickID() = 0;
    QString getSDLPlatform();
#endif
    virtual bool isGameController();
    virtual bool isKnownController();

    void setButtonName(int index, QString tempName);
    void setAxisButtonName(int axisIndex, int buttonIndex, QString tempName);
    void setStickButtonName(int stickIndex, int buttonIndex, QString tempName);
    void setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName);
    void setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName);

    void setAxisName(int axisIndex, QString tempName);
    void setStickName(int stickIndex, QString tempName);
    void setDPadName(int dpadIndex, QString tempName);
    void setVDPadName(int vdpadIndex, QString tempName);

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
    bool hasCalibrationThrottle(int axisNum);
    JoyAxis::ThrottleTypes getCalibrationThrottle(int axisNum);
    void setCalibrationThrottle(int axisNum, JoyAxis::ThrottleTypes throttle);
    void setCalibrationStatus(int axisNum, JoyAxis::ThrottleTypes throttle);
    void removeCalibrationStatus(int axisNum);

    void sendLoadProfileRequest(QString location);
    AntiMicroSettings *getSettings();

    void activatePossiblePendingEvents();
    void activatePossibleControlStickEvents();
    void activatePossibleAxisEvents();
    void activatePossibleDPadEvents();
    void activatePossibleVDPadEvents();
    void activatePossibleButtonEvents();

    bool isEmptyGUID(QString tempGUID);
    bool isRelevantGUID(QString tempGUID);

    void setRawAxisDeadZone(int deadZone);
    int getRawAxisDeadZone();
    void rawAxisEvent(int index, int value);

    static const int NUMBER_JOYSETS;
    static const int DEFAULTKEYPRESSTIME;
    static const unsigned int DEFAULTKEYREPEATDELAY;
    static const unsigned int DEFAULTKEYREPEATRATE;
    static const int RAISEDDEADZONE;

protected:
    void enableSetConnections(SetJoystick *setstick);
    bool elementsHaveNames();

    SDL_Joystick* joyhandle;
    QHash<int, SetJoystick*> joystick_sets;
    QHash<int, JoyAxis::ThrottleTypes> cali;
    AntiMicroSettings *settings;
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

    QList<bool> buttonstates;
    QList<int> axesstates;
    QList<int> dpadstates;

    int rawAxisDeadZone;

    static QRegExp emptyGUID;

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
    void rawAxisMoved(int axis, int value);
    void profileUpdated();
    void propertyUpdated();
    void profileNameEdited(QString text);
    void requestProfileLoad(QString location);
    void requestWait();

public slots:
    void reset();
    void transferReset();
    void reInitButtons();
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
    void haltServices();
    void finalRemoval();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

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
Q_DECLARE_METATYPE(SDL_JoystickID)

#endif // INPUTDEVICE_H
