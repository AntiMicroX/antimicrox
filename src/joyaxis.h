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

#ifndef JOYAXIS_H
#define JOYAXIS_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttontypes/joyaxisbutton.h"

class JoyControlStick;

class JoyAxis : public QObject
{
    Q_OBJECT
public:
    explicit JoyAxis(int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    ~JoyAxis();

    enum ThrottleTypes {
        NegativeHalfThrottle = -2,
        NegativeThrottle = -1,
        NormalThrottle = 0,
        PositiveThrottle = 1,
        PositiveHalfThrottle = 2
    };

    void joyEvent(int value, bool ignoresets=false, bool updateLastValues=true);
    void queuePendingEvent(int value, bool ignoresets=false, bool updateLastValues=true);
    void activatePendingEvent();
    bool hasPendingEvent();
    void clearPendingEvent();

    bool inDeadZone(int value);

    virtual QString getName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();

    void setIndex(int index);
    int getIndex();
    int getRealJoyIndex();

    JoyAxisButton *getPAxisButton();
    JoyAxisButton *getNAxisButton();

    int getDeadZone();

    int getMaxZoneValue();
    void setThrottle(int value);
    void setInitialThrottle(int value);
    int getThrottle();
    int getCurrentThrottledValue();
    int getCurrentRawValue();
    //int getCurrentThrottledMin();
    //int getCurrentThrottledMax();
    int getCurrentThrottledDeadValue();
    int getCurrentlyAssignedSet();
    JoyAxisButton* getAxisButtonByValue(int value);

    double getDistanceFromDeadZone();
    double getDistanceFromDeadZone(int value);
    double getRawDistance(int value);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    void setControlStick(JoyControlStick *stick);
    void removeControlStick(bool performRelease = true);
    bool isPartControlStick();
    JoyControlStick* getControlStick();
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

    virtual QString getAxisName();
    virtual int getDefaultDeadZone();
    virtual int getDefaultMaxZone();
    virtual ThrottleTypes getDefaultThrottle();

    virtual void setDefaultAxisName(QString tempname);
    virtual QString getDefaultAxisName();

    SetJoystick* getParentSet();

    virtual bool isDefault();

    bool isRelativeSpring();
    void copyAssignments(JoyAxis *destAxis);

    int getLastKnownThrottleValue();
    int getLastKnownRawValue();
    int getProperReleaseValue();

    // Don't use direct assignment but copying from a current axis.
    void copyRawValues(JoyAxis *srcAxis);
    void copyThrottledValues(JoyAxis *srcAxis);

    void setExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyExtraAccelerationCurve getExtraAccelerationCurve();

    virtual void eventReset();

    // Define default values for many properties.
    static const int AXISMIN;
    static const int AXISMAX;
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;
    static const ThrottleTypes DEFAULTTHROTTLE;

    static const float JOYSPEED;

    static const QString xmlName;

protected:
    void createDeskEvent(bool ignoresets = false);
    void adjustRange();
    int calculateThrottledValue(int value);
    void setCurrentRawValue(int value);
    void performCalibration(int value);
    void stickPassEvent(int value, bool ignoresets=false, bool updateLastValues=true);

    virtual bool readMainConfig(QXmlStreamReader *xml);
    virtual bool readButtonConfig(QXmlStreamReader *xml);

    int index;
    int deadZone;
    int maxZoneValue;
    bool isActive;

    JoyAxisButton *paxisbutton;
    JoyAxisButton *naxisbutton;

    bool eventActive;
    int currentThrottledValue;
    int currentRawValue;
    int throttle;
    JoyAxisButton *activeButton;
    int originset;

    int currentThrottledDeadValue;
    JoyControlStick *stick;
    QString axisName;
    QString defaultAxisName;
    SetJoystick *parentSet;
    int lastKnownThottledValue;
    int lastKnownRawValue;

    int pendingValue;
    bool pendingEvent;
    bool pendingIgnoreSets;
    // TODO: CHECK IF PROPERTY IS NEEDED.
    //bool pendingUpdateLastValues;

signals:
    void active(int value);
    void released(int value);
    void moved(int value);
    void throttleChangePropogated(int index);
    void throttleChanged();
    void axisNameChanged();
    void propertyUpdated();

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
};

#endif // JOYAXIS_H
