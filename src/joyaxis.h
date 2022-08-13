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

#ifndef JOYAXIS_H
#define JOYAXIS_H

#include <QList>
#include <QObject>

#include "haptictriggermodeps5.h"
#include "joybuttontypes/joyaxisbutton.h"

class HapticTriggerPs5;
class JoyControlStick;
class SetJoystick;
class JoyAxisButton;
class QXmlStreamReader;
class QXmlStreamWriter;
class JoyAxis;

/**
 * @brief Represents single axis of of joystick (or other input)
 *
 */
class JoyAxis : public QObject
{
    Q_OBJECT

  public:
    explicit JoyAxis(int index, int originset, SetJoystick *parentSet, QObject *parent);
    virtual ~JoyAxis();

    enum ThrottleTypes
    {
        NegativeHalfThrottle = -2,
        NegativeThrottle = -1,
        NormalThrottle = 0,
        PositiveThrottle = 1,
        PositiveHalfThrottle = 2
    };

    void joyEvent(int value, bool ignoresets = false, bool updateLastValues = true);          // JoyAxisEvent class
    void queuePendingEvent(int value, bool ignoresets = false, bool updateLastValues = true); // JoyAxisEvent class
    void activatePendingEvent();                                                              // JoyAxisEvent class
    bool hasPendingEvent();                                                                   // JoyAxisEvent class
    void clearPendingEvent();                                                                 // JoyAxisEvent class
    bool inDeadZone(int value);

    virtual QString getName(bool forceFullFormat = false, bool displayNames = false);
    virtual QString getPartialName(bool forceFullFormat = false, bool displayNames = false);
    virtual QString getXmlName(); // JoyAxisXml class

    void setIndex(int index);
    int getIndex();
    int getRealJoyIndex();

    JoyAxisButton *getPAxisButton();
    JoyAxisButton *getNAxisButton();

    int getDeadZone();

    int getMaxZoneValue();
    void setThrottle(int value);
    void setInitialThrottle(int value);
    void updateCurrentThrottledValue(int newValue);
    int getThrottle();
    int getCurrentThrottledValue();
    int getCurrentRawValue();
    int getCurrentThrottledDeadValue();
    int getCurrentlyAssignedSet();
    JoyAxisButton *getAxisButtonByValue(int value);

    double getDistanceFromDeadZone();
    double getDistanceFromDeadZone(int value);
    double getRawDistance(int value);

    void setControlStick(JoyControlStick *stick);
    void removeControlStick(bool performRelease = true);
    bool isPartControlStick();
    JoyControlStick *getControlStick();
    bool hasControlOfButtons();
    void removeVDPads();

    void setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode);
    bool hasSameButtonsMouseMode();
    JoyButton::JoyMouseMovementMode getButtonsPresetMouseMode();

    void setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve);
    bool hasSameButtonsMouseCurve();
    JoyButton::JoyMouseCurve getButtonsPresetMouseCurve();

    void setButtonsSpringWidth(int value);
    int getButtonsPresetSpringWidth();

    void setButtonsSpringHeight(int value);
    int getButtonsPresetSpringHeight();

    void setButtonsSensitivity(double value);
    double getButtonsPresetSensitivity();

    void setButtonsWheelSpeedX(int value);
    void setButtonsWheelSpeedY(int value);

    double getButtonsEasingDuration();

    bool isCalibrated() const;
    void resetCalibration();
    void getCalibration(double *offset, double *gain) const;
    void setCalibration(double offset, double gain);

    virtual QString getAxisName();
    virtual int getDefaultDeadZone();
    virtual int getDefaultMaxZone();
    virtual ThrottleTypes getDefaultThrottle();

    virtual void setDefaultAxisName(QString tempname);
    virtual QString getDefaultAxisName();

    SetJoystick *getParentSet();

    virtual bool isDefault();

    bool isRelativeSpring();
    void copyAssignments(JoyAxis *destAxis);

    int getLastKnownThrottleValue();
    int getLastKnownRawValue();
    int getProperReleaseValue();
    void setCurrentRawValue(int value);

    // Don't use direct assignment but copying from a current axis.
    void copyRawValues(JoyAxis *srcAxis);
    void copyThrottledValues(JoyAxis *srcAxis);

    void setExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyExtraAccelerationCurve getExtraAccelerationCurve();

    virtual void eventReset(); // JoyAxisEvent class

    static const ThrottleTypes DEFAULTTHROTTLE;
    int calculateThrottledValue(int value);

    virtual bool hasHapticTrigger() const;
    virtual HapticTriggerPs5 *getHapticTrigger() const;
    virtual void setHapticTriggerMode(HapticTriggerModePs5);

  protected:
    void createDeskEvent(bool ignoresets = false); // JoyAxisEvent class
    void adjustRange();

    void performCalibration(int value);
    void stickPassEvent(int value, bool ignoresets = false, bool updateLastValues = true); // JoyAxisEvent class

    JoyAxisButton *paxisbutton;
    JoyAxisButton *naxisbutton;

    QString axisName;
    QString defaultAxisName;

    int throttle;
    int deadZone;
    int maxZoneValue;
    int currentRawValue;
    int currentThrottledValue;
    int currentThrottledDeadValue;
    int m_index;
    int lastKnownThottledValue;
    int lastKnownRawValue;
    int pendingValue;

    bool isActive;
    bool pendingEvent;
    bool pendingIgnoreSets;
    bool eventActive;

    JoyAxisButton *activeButton;

    // TODO: CHECK IF PROPERTY IS NEEDED.
    // bool pendingUpdateLastValues;

  signals:
    void active(int value);
    void released(int value);
    void moved(int value);
    void throttleChangePropogated(int index);
    void throttleChanged();
    void axisNameChanged();
    void propertyUpdated();
    void hapticTriggerChanged();

  public slots:
    virtual void reset();
    virtual void reset(int index);
    void propogateThrottleChange();

    void setDeadZone(int value);
    void setMaxZoneValue(int value);
    void setAxisName(QString tempName);
    void setButtonsSpringRelativeStatus(bool value);
    void setButtonsEasingDuration(double value);

    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

  private:
    int m_originset;

    JoyControlStick *m_stick;

    SetJoystick *m_parentSet;

    bool m_calibrated;
    double m_offset;
    double m_gain;

    void resetPrivateVars();
};

#endif // JOYAXIS_H
