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

#ifndef SETJOYSTICK_H
#define SETJOYSTICK_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

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
    unsigned int getRealIndex();
    virtual void refreshButtons ();
    virtual void refreshAxes();
    virtual void refreshHats();
    void release();
    void addControlStick(int index, JoyControlStick *stick);
    void removeControlStick(int index);
    void addVDPad(int index, VDPad *vdpad);
    void removeVDPad(int index);
    void setIgnoreEventState(bool ignore);

    InputDevice* getInputDevice();

    void setName(QString name);
    QString getName();
    QString getSetLabel();

    void raiseAxesDeadZones(int deadZone=0);
    void currentAxesDeadZones(QList<int> *axesDeadZones);
    void setAxesDeadZones(QList<int> *axesDeadZones);
    void setAxisThrottle(int axisNum, JoyAxis::ThrottleTypes throttle);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int MAXNAMELENGTH;
    static const int RAISEDDEADZONE;

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
    InputDevice *device;
    QString name;

signals:
    void setChangeActivated(int index);
    void setAssignmentButtonChanged(int button, int originset, int newset, int mode);
    void setAssignmentAxisChanged(int button, int axis, int originset, int newset, int mode);
    void setAssignmentStickChanged(int button, int stick, int originset, int newset, int mode);
    void setAssignmentDPadChanged(int button, int dpad, int originset, int newset, int mode);
    void setAssignmentVDPadChanged(int button, int dpad, int originset, int newset, int mode);
    void setAssignmentAxisThrottleChanged(int axis, int originset);
    void setButtonClick(int index, int button);
    void setButtonRelease(int index, int button);
    void setAxisButtonClick(int setindex, int axis, int button);
    void setAxisButtonRelease(int setindex, int axis, int button);
    void setAxisActivated(int setindex, int axis, int value);
    void setAxisReleased(int setindex, int axis, int value);
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
    void propertyUpdated();
    
public slots:
    virtual void reset();
    void copyAssignments(SetJoystick *destSet);
    void propogateSetChange(int index);
    void propogateSetButtonAssociation(int button, int newset, int mode);
    void propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode);
    void propogateSetStickButtonAssociation(int button, int stick, int newset, int mode);
    void propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode);
    void propogateSetVDPadButtonAssociation(int button, int dpad, int newset, int mode);
    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

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
    void propogateSetAxisReleased(int value);

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

Q_DECLARE_METATYPE(SetJoystick*)

#endif // SETJOYSTICK_H
