/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "joyaxis.h"
#include "joysensordirection.h"
#include "joysensortype.h"
#include "xml/setjoystickxml.h"

class InputDevice;
class JoyButton;
class JoyDPad;
class JoyControlStick;
class JoySensor;
class VDPad;

/**
 * @brief A set of mapped events which can by switched by a controller event.
 *  Contains controller input objects like axes or buttons and their mappings,
 *  and forwards some QT GUI events.
 */
class SetJoystick : public SetJoystickXml
{
    Q_OBJECT

  public:
    explicit SetJoystick(InputDevice *device, int index, QObject *parent);
    explicit SetJoystick(InputDevice *device, int index, bool runreset, QObject *parent);
    ~SetJoystick();

    JoyAxis *getJoyAxis(int index) const;
    JoyButton *getJoyButton(int index) const;
    JoyDPad *getJoyDPad(int index) const;
    JoyControlStick *getJoyStick(int index) const;
    JoySensor *getSensor(JoySensorType type) const;
    VDPad *getVDPad(int index) const;

    int getNumberButtons() const;
    int getNumberAxes() const;
    int getNumberHats() const;
    int getNumberSticks() const;
    bool hasSensor(JoySensorType type) const;
    int getNumberVDPads() const;

    QHash<int, JoyButton *> const &getButtons() const;
    QHash<int, JoyDPad *> const &getHats() const;
    QHash<int, JoyControlStick *> const &getSticks() const;
    QHash<JoySensorType, JoySensor *> const &getSensors() const;
    QHash<int, VDPad *> const &getVdpads() const;
    QHash<int, JoyAxis *> *getAxes();

    int getIndex() const;
    int getRealIndex() const;
    virtual void refreshButtons(); // SetButton class
    virtual void refreshAxes();    // SetAxis class
    virtual void refreshHats();    // SetHat class
    virtual void refreshSensors();
    void release();
    void addControlStick(int index, JoyControlStick *stick); // SetStick class
    void removeControlStick(int index);                      // SetStick class
    void addVDPad(int index, VDPad *vdpad);                  // SetVDPad class
    void removeVDPad(int index);                             // SetVDPad class
    void setIgnoreEventState(bool ignore);

    InputDevice *getInputDevice() const;

    void setName(QString name);
    QString getName() const;
    QString getSetLabel();

    void raiseAxesDeadZones(int deadZone = 0);                          // SetAxis class
    void currentAxesDeadZones(QList<int> *axesDeadZones);               // SetAxis class
    void setAxesDeadZones(QList<int> *axesDeadZones);                   // SetAxis class
    void setAxisThrottle(int axisNum, JoyAxis::ThrottleTypes throttle); // SetAxis class
    QList<JoyButton *> const &getLastClickedButtons() const;
    void removeAllBtnFromQueue();
    int getCountBtnInList(QString partialName);
    bool isSetEmpty();

  protected:
    void deleteButtons(); // SetButton class
    void deleteAxes();    // SetAxis class
    void deleteHats();    // SetHat class
    void deleteSticks();  // SetStick class
    void deleteSensors();
    void deleteVDpads(); // SetVDPad class

    void enableButtonConnections(JoyButton *button); // SetButton class
    void enableAxisConnections(JoyAxis *axis);       // SetAxis class
    void enableHatConnections(JoyDPad *dpad);        // SetHat class
    void enableSensorConnections(JoySensor *sensor);

  signals:
    void setChangeActivated(int index);
    void setAssignmentButtonChanged(int button, int originset, int newset, int mode);           // SetButton class
    void setAssignmentAxisChanged(int button, int axis, int originset, int newset, int mode);   // SetAxis class
    void setAssignmentStickChanged(int button, int stick, int originset, int newset, int mode); // SetStick class
    void setAssignmentSensorChanged(JoySensorDirection direction, JoySensorType sensor, int originset, int newset, int mode);
    void setAssignmentDPadChanged(int button, int dpad, int originset, int newset, int mode);  // SetHat class
    void setAssignmentVDPadChanged(int button, int dpad, int originset, int newset, int mode); // SetVDPad class
    void setAssignmentAxisThrottleChanged(int axis, int originset);                            // SetAxis class
    void setButtonClick(int index, int button);                                                // SetButton class
    void setButtonRelease(int index, int button);                                              // SetButton class
    void setAxisButtonClick(int setindex, int axis, int button);                               // SetAxis class
    void setAxisButtonRelease(int setindex, int axis, int button);                             // SetAxis class
    void setAxisActivated(int setindex, int axis, int value);                                  // SetAxis class
    void setAxisReleased(int setindex, int axis, int value);                                   // SetAxis class
    void setStickButtonClick(int setindex, int stick, int button);                             // SetStick class
    void setStickButtonRelease(int setindex, int stick, int button);                           // SetStick class
    void setSensorActivated(int setindex, JoySensorType type, int value);
    void setSensorReleased(int setindex, JoySensorType, int value);
    void setSensorButtonClick(int setindex, JoySensorType type, JoySensorDirection direction);
    void setSensorButtonRelease(int setindex, JoySensorType type, JoySensorDirection direction);
    void setDPadButtonClick(int setindex, int dpad, int button);   // SetHat class
    void setDPadButtonRelease(int setindex, int dpad, int button); // SetHat class

    void setButtonNameChange(int index);                            // SetButton class
    void setAxisButtonNameChange(int axisIndex, int buttonIndex);   // SetAxis class
    void setStickButtonNameChange(int stickIndex, int buttonIndex); // SetStick class
    void setSensorButtonNameChange(JoySensorType type, JoySensorDirection direction);
    void setDPadButtonNameChange(int dpadIndex, int buttonIndex);   // SetHat class
    void setVDPadButtonNameChange(int vdpadIndex, int buttonIndex); // SetVDPad class

    void setAxisNameChange(int axisIndex);   // SetAxis class
    void setStickNameChange(int stickIndex); // SetStick class
    void setSensorNameChange(JoySensorType type);
    void setDPadNameChange(int dpadIndex);   // SetHat class
    void setVDPadNameChange(int vdpadIndex); // SetVDPad class
    void propertyUpdated();

  public slots:
    virtual void reset();
    void copyAssignments(SetJoystick *destSet);
    void propogateSetChange(int index);
    void propogateSetButtonAssociation(int button, int newset, int mode);                 // SetButton class
    void propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode);   // SetAxis class
    void propogateSetStickButtonAssociation(int button, int stick, int newset, int mode); // SetStick class
    void propagateSetSensorButtonAssociation(JoySensorDirection direction, JoySensorType sensor, int newset, int mode);
    void propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode);  // SetHat class
    void propogateSetVDPadButtonAssociation(int button, int dpad, int newset, int mode); // SetVDPad class
    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

  protected slots:
    void propogateSetAxisThrottleSetting(int index); // SetAxis class
    void propogateSetButtonClick(int button);        // SetButton class
    void propogateSetButtonRelease(int button);      // SetButton class
    void propogateSetAxisButtonClick(int button);    // SetAxis class
    void propogateSetAxisButtonRelease(int button);  // SetAxis class
    void propogateSetStickButtonClick(int button);   // SetStick class
    void propogateSetStickButtonRelease(int button); // SetStick class
    void propagateSetSensorButtonClick(int button);
    void propagateSetSensorButtonRelease(int button);
    void propogateSetDPadButtonClick(int button);   // SetHat class
    void propogateSetDPadButtonRelease(int button); // SetHat class
    void propogateSetAxisActivated(int value);      // SetAxis class
    void propogateSetAxisReleased(int value);       // SetAxis class

    void propogateSetButtonNameChange();      // SetButton class
    void propogateSetAxisButtonNameChange();  // SetAxis class
    void propogateSetStickButtonNameChange(); // SetStick class
    void propagateSetSensorButtonNameChange();
    void propogateSetDPadButtonNameChange();  // SetHat class
    void propogateSetVDPadButtonNameChange(); // SetVDPad class

    void propogateSetAxisNameChange();  // SetAxis class
    void propogateSetStickNameChange(); // SetStick class
    void propagateSetSensorNameChange();
    void propogateSetDPadNameChange();  // SetHat class
    void propogateSetVDPadNameChange(); // SetVDPad class

  private:
    QHash<int, JoyButton *> m_buttons;
    QHash<int, JoyAxis *> axes;
    QHash<int, JoyDPad *> hats;
    QHash<int, JoyControlStick *> sticks;
    QHash<JoySensorType, JoySensor *> m_sensors;
    QHash<int, VDPad *> vdpads;

    QList<JoyButton *> lastClickedButtons;

    int m_index;
    InputDevice *m_device;
    QString m_name;
};

Q_DECLARE_METATYPE(SetJoystick *)

#endif // SETJOYSTICK_H
