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

#ifndef JOYCONTROLSTICK_H
#define JOYCONTROLSTICK_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joyaxis.h"
#include "joybutton.h"
#include "joycontrolstickdirectionstype.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"

class JoyControlStick : public QObject, public JoyStickDirectionsType
{
    Q_OBJECT
public:
    explicit JoyControlStick(JoyAxis *axisX, JoyAxis *axisY,
                             int index, int originset = 0, QObject *parent = 0);
    ~JoyControlStick();

    enum JoyMode {StandardMode=0, EightWayMode, FourWayCardinal, FourWayDiagonal};

    void joyEvent(bool ignoresets=false);

    bool inDeadZone();
    int getDeadZone();
    int getDiagonalRange();

    double getDistanceFromDeadZone();
    double getDistanceFromDeadZone(int axisXValue, int axisYValue);

    double getAbsoluteRawDistance();
    double getAbsoluteRawDistance(int axisXValue, int axisYValue);
    double getNormalizedAbsoluteDistance();

    int getIndex();
    void setIndex(int index);
    int getRealJoyIndex();
    int getMaxZone();
    int getCurrentlyAssignedSet();
    virtual QString getName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    JoyStickDirections getCurrentDirection();
    int getXCoordinate();
    int getYCoordinate();
    int getCircleXCoordinate();
    int getCircleYCoordinate();

    double calculateBearing();
    double calculateBearing(int axisXValue, int axisYValue);

    QList<double> getDiagonalZoneAngles();
    QList<int> getFourWayCardinalZoneAngles();
    QList<int> getFourWayDiagonalZoneAngles();
    QHash<JoyStickDirections, JoyControlStickButton*>* getButtons();
    JoyControlStickModifierButton* getModifierButton();
    JoyAxis* getAxisX();
    JoyAxis* getAxisY();

    void replaceXAxis(JoyAxis *axis);
    void replaceYAxis(JoyAxis *axis);
    void replaceAxes(JoyAxis *axisX, JoyAxis* axisY);

    JoyControlStickButton* getDirectionButton(JoyStickDirections direction);
    double calculateMouseDirectionalDistance(JoyControlStickButton *button);

    double calculateDirectionalDistance();
    double calculateLastDirectionalDistance();

    double calculateLastMouseDirectionalDistance(JoyControlStickButton *button);


    double calculateLastAccelerationButtonDistance(JoyControlStickButton *button);
    double calculateAccelerationDistance(JoyControlStickButton *button);
    double calculateXAxisDistance(int axisXValue);
    double calculateYAxisDistance(int axisYValue);
    double calculateLastAccelerationDirectionalDistance();
    double getRadialDistance(int axisXValue, int axisYValue);

    void setJoyMode(JoyMode mode);
    JoyMode getJoyMode();

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

    void setButtonsExtraAccelerationStatus(bool enabled);
    bool getButtonsExtraAccelerationStatus();

    void setButtonsExtraAccelerationMultiplier(double value);
    double getButtonsExtraAccelerationMultiplier();

    void setButtonsStartAccelerationMultiplier(double value);
    double getButtonsStartAccelerationMultiplier();

    void setButtonsMinAccelerationThreshold(double value);
    double getButtonsMinAccelerationThreshold();

    void setButtonsMaxAccelerationThreshold(double value);
    double getButtonsMaxAccelerationThreshold();

    void setButtonsAccelerationExtraDuration(double value);
    double getButtonsAccelerationEasingDuration();

    void setButtonsSpringDeadCircleMultiplier(int value);
    int getButtonsSpringDeadCircleMultiplier();

    void setButtonsExtraAccelCurve(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyExtraAccelerationCurve getButtonsExtraAccelerationCurve();

    void releaseButtonEvents();
    QString getStickName();

    virtual bool isDefault();

    virtual void setDefaultStickName(QString tempname);
    virtual QString getDefaultStickName();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    SetJoystick* getParentSet();
    bool hasSlotsAssigned();

    bool isRelativeSpring();
    void copyAssignments(JoyControlStick *destStick);

    double getCircleAdjust();
    unsigned int getStickDelay();

    double getButtonsEasingDuration();

    void queueJoyEvent(bool ignoresets);
    bool hasPendingEvent();
    void activatePendingEvent();
    void clearPendingEvent();
    //double calculateXDistanceFromDeadZone(int axisXValue, int axisYValue, bool interpolate=false);

    double getSpringDeadCircleX();
    double getSpringDeadCircleY();

    QHash<JoyStickDirections, JoyControlStickButton*> getButtonsForDirection(JoyControlStick::JoyStickDirections direction);
    void setDirButtonsUpdateInitAccel(JoyControlStick::JoyStickDirections direction, bool state);

    static const double PI;

    // Define default values for stick properties.
    static const int DEFAULTDEADZONE;
    static const int DEFAULTMAXZONE;
    static const int DEFAULTDIAGONALRANGE;
    static const JoyMode DEFAULTMODE;
    static const double DEFAULTCIRCLE;
    static const unsigned int DEFAULTSTICKDELAY;


protected:
    virtual void populateButtons();
    void createDeskEvent(bool ignoresets = false);

    void determineStandardModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2);
    void determineEightWayModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2, JoyControlStickButton *&eventbutton3);
    void determineFourWayCardinalEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2);
    void determineFourWayDiagonalEvent(JoyControlStickButton *&eventbutton3);

    JoyControlStick::JoyStickDirections determineStandardModeDirection();
    JoyControlStick::JoyStickDirections determineStandardModeDirection(int axisXValue, int axisYValue);

    JoyControlStick::JoyStickDirections determineEightWayModeDirection();
    JoyControlStick::JoyStickDirections determineEightWayModeDirection(int axisXValue, int axisYValue);

    JoyControlStick::JoyStickDirections determineFourWayCardinalDirection();
    JoyControlStick::JoyStickDirections determineFourWayCardinalDirection(int axisXValue, int axisYValue);

    JoyControlStick::JoyStickDirections determineFourWayDiagonalDirection();
    JoyControlStick::JoyStickDirections determineFourWayDiagonalDirection(int axisXValue, int axisYValue);

    JoyControlStick::JoyStickDirections calculateStickDirection();
    JoyControlStick::JoyStickDirections calculateStickDirection(int axisXValue, int axisYValue);

    void performButtonPress(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets);
    void performButtonRelease(JoyControlStickButton *&eventbutton, bool ignoresets);

    void refreshButtons();
    void deleteButtons();
    void resetButtons();

    double calculateXDistanceFromDeadZone(bool interpolate=false);
    double calculateXDistanceFromDeadZone(int axisXValue, int axisYValue, bool interpolate=false);

    double calculateYDistanceFromDeadZone(bool interpolate=false);
    double calculateYDistanceFromDeadZone(int axisXValue, int axisYValue, bool interpolate=false);

    int calculateCircleXValue(int axisXValue, int axisYValue);
    int calculateCircleYValue(int axisXValue, int axisYValue);

    double calculateEightWayDiagonalDistanceFromDeadZone();
    double calculateEightWayDiagonalDistanceFromDeadZone(int axisXValue, int axisYValue);
    double calculateEightWayDiagonalDistance(int axisXValue, int axisYValue);

    inline double calculateXDiagonalDeadZone(int axisXValue, int axisYValue);
    inline double calculateYDiagonalDeadZone(int axisXValue, int axisYValue);

    QHash<JoyStickDirections, JoyControlStickButton*> getApplicableButtons();
    void clearPendingAxisEvents();

    JoyAxis *axisX;
    JoyAxis *axisY;
    int originset;
    int deadZone;
    int diagonalRange;
    int maxZone;
    bool isActive;
    JoyControlStickButton *activeButton1;
    JoyControlStickButton *activeButton2;
    JoyControlStickButton *activeButton3;
    bool safezone;
    int index;
    JoyStickDirections currentDirection;
    JoyMode currentMode;
    QString stickName;
    QString defaultStickName;
    double circle;
    QTimer directionDelayTimer;
    unsigned int stickDelay;
    bool pendingStickEvent;

    QHash<JoyStickDirections, JoyControlStickButton*> buttons;
    JoyControlStickModifierButton *modifierButton;

signals:
    void moved(int xaxis, int yaxis);
    void active(int xaxis, int yaxis);
    void released(int axis, int yaxis);
    void deadZoneChanged(int value);
    void diagonalRangeChanged(int value);
    void maxZoneChanged(int value);
    void circleAdjustChange(double circle);
    void stickDelayChanged(int value);
    void stickNameChanged();
    void joyModeChanged();
    void propertyUpdated();

public slots:
    void reset();
    void setDeadZone(int value);
    void setMaxZone(int value);
    void setDiagonalRange(int value);
    void setStickName(QString tempName);
    void setButtonsSpringRelativeStatus(bool value);
    void setCircleAdjust(double circle);
    void setStickDelay(int value);
    void setButtonsEasingDuration(double value);

    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

private slots:
    void stickDirectionChangeEvent();
};

#endif // JOYCONTROLSTICK_H
