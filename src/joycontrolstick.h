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

#ifndef JOYCONTROLSTICK_H
#define JOYCONTROLSTICK_H

#include "joybuttontypes/joybutton.h"
#include "joycontrolstickdirectionstype.h"

#include <QPointer>

class JoyAxis;
class JoyControlStickButton;
class JoyControlStickModifierButton;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Represents stick of a joystick
 *
 */
class JoyControlStick : public QObject, public JoyStickDirectionsType
{
    Q_OBJECT

  public:
    explicit JoyControlStick(JoyAxis *axisX, JoyAxis *axisY, int index, int originset, QObject *parent);
    ~JoyControlStick();

    enum JoyMode
    {
        StandardMode = 0,
        EightWayMode,
        FourWayCardinal,
        FourWayDiagonal
    };

    void joyEvent(bool ignoresets = false); // JoyControlStickEvent class
    void setIndex(int index);
    void replaceXAxis(JoyAxis *axis);                 // JoyControlStickAxes class
    void replaceYAxis(JoyAxis *axis);                 // JoyControlStickAxes class
    void replaceAxes(JoyAxis *axisX, JoyAxis *axisY); // JoyControlStickAxes class
    void releaseButtonEvents();                       // JoyControlStickEvent class
    void copyAssignments(JoyControlStick *destStick);
    void queueJoyEvent(bool ignoresets); // JoyControlStickEvent class
    void activatePendingEvent();         // JoyControlStickEvent class
    void clearPendingEvent();            // JoyControlStickEvent class

    bool inDeadZone();
    bool hasSlotsAssigned();
    bool isRelativeSpring();
    bool hasPendingEvent(); // JoyControlStickEvent class

    bool isCalibrated() const;
    void resetCalibration();
    void getCalibration(double *offsetX, double *gainX, double *offsetY, double *gainY) const;
    void setCalibration(double offsetX, double gainX, double offsetY, double gainY);

    int getDeadZone();
    int getDiagonalRange();
    int getIndex();
    int getRealJoyIndex();
    int getMaxZone();
    int getModifierZone() const;
    bool getModifierZoneInverted() const;
    int getCurrentlyAssignedSet();
    int getXCoordinate();
    int getYCoordinate();
    int getCircleXCoordinate();
    int getCircleYCoordinate();
    int getStickDelay();

    double getDistanceFromDeadZone();                                              // JoyControlStickAxes class
    double getDistanceFromDeadZone(int axisXValue, int axisYValue);                // JoyControlStickAxes class
    double getAbsoluteRawDistance();                                               // JoyControlStickAxes class
    double getAbsoluteRawDistance(int axisXValue, int axisYValue);                 // JoyControlStickAxes class
    double getNormalizedAbsoluteDistance();                                        // JoyControlStickAxes class
    double calculateBearing();                                                     // JoyControlStickAxes class
    double calculateBearing(int axisXValue, int axisYValue);                       // JoyControlStickAxes class
    double calculateMouseDirectionalDistance(JoyControlStickButton *button);       // JoyControlStickAxes class
    double calculateDirectionalDistance();                                         // JoyControlStickAxes class
    double calculateLastDirectionalDistance();                                     // JoyControlStickAxes class
    double calculateLastMouseDirectionalDistance(JoyControlStickButton *button);   // JoyControlStickAxes class
    double calculateLastAccelerationButtonDistance(JoyControlStickButton *button); // JoyControlStickAxes class
    double calculateAccelerationDistance(JoyControlStickButton *button);           // JoyControlStickAxis class
    double calculateXAxisDistance(int axisXValue);                                 // JoyControlStickAxis class
    double calculateYAxisDistance(int axisYValue);                                 // JoyControlStickAxis class
    double calculateLastAccelerationDirectionalDistance();                         // JoyControlStickAxis class
    double getRadialDistance(int axisXValue, int axisYValue);                      // JoyControlStickAxis class
    double getCircleAdjust();                                                      // JoyControlStickAxis class
    double getButtonsEasingDuration();
    double getSpringDeadCircleX(); // JoyControlStickAxis class
    double getSpringDeadCircleY(); // JoyControlStickAxis class

    QString getStickName();

    virtual QString getName(bool forceFullFormat = false, bool displayNames = false);
    virtual QString getPartialName(bool forceFullFormat = false, bool displayNames = false);

    JoyStickDirections getCurrentDirection(); // JoyControlStickAxes class

    QList<double> getDiagonalZoneAngles();     // JoyControlStickAxes class
    QList<int> getFourWayCardinalZoneAngles(); // JoyControlStickAxes class
    QList<int> getFourWayDiagonalZoneAngles(); // JoyControlStickAxes class
    QHash<JoyStickDirections, JoyControlStickButton *> *getButtons();

    JoyControlStickButton *getDirectionButton(JoyStickDirections direction); // JoyControlStickAxes class
    JoyControlStickModifierButton *getModifierButton();
    SetJoystick *getParentSet();
    JoyAxis *getAxisX(); // JoyControlStickAxes class
    JoyAxis *getAxisY(); // JoyControlStickAxes class

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

    QHash<JoyStickDirections, JoyControlStickButton *>
    getButtonsForDirection(JoyControlStick::JoyStickDirections direction); // JoyControlStickAxes class
    void setDirButtonsUpdateInitAccel(JoyControlStick::JoyStickDirections direction,
                                      bool state); // JoyControlStickAxes class

    double calculateXDiagonalDeadZone(int axisXValue, int axisYValue); // JoyControlStickAxes class
    double calculateYDiagonalDeadZone(int axisXValue, int axisYValue); // JoyControlStickAxes class

    virtual bool isDefault();
    virtual void setDefaultStickName(QString tempname);
    virtual QString getDefaultStickName();
    virtual void readConfig(QXmlStreamReader *xml);  // JoyControlStickXml class
    virtual void writeConfig(QXmlStreamWriter *xml); // JoyControlStickXml class

    static const JoyMode DEFAULTMODE;

  protected:
    virtual void populateButtons();

    void createDeskEvent(bool ignoresets = false); // JoyControlStickEvent class
    void determineStandardModeEvent(JoyControlStickButton *&eventbutton1,
                                    JoyControlStickButton *&eventbutton2); // JoyControlStickEvent class
    void determineEightWayModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2,
                                    JoyControlStickButton *&eventbutton3); // JoyControlStickEvent class
    void determineFourWayCardinalEvent(JoyControlStickButton *&eventbutton1,
                                       JoyControlStickButton *&eventbutton2); // JoyControlStickEvent class
    void determineFourWayDiagonalEvent(JoyControlStickButton *&eventbutton3); // JoyControlStickEvent class

    JoyControlStick::JoyStickDirections determineStandardModeDirection(); // JoyControlStickAxes class
    JoyControlStick::JoyStickDirections determineStandardModeDirection(int axisXValue,
                                                                       int axisYValue); // JoyControlStickAxes class

    JoyControlStick::JoyStickDirections determineEightWayModeDirection(); // JoyControlStickAxes class
    JoyControlStick::JoyStickDirections determineEightWayModeDirection(int axisXValue,
                                                                       int axisYValue); // JoyControlStickAxes class

    JoyControlStick::JoyStickDirections determineFourWayCardinalDirection(); // JoyControlStickAxes class
    JoyControlStick::JoyStickDirections determineFourWayCardinalDirection(int axisXValue,
                                                                          int axisYValue); // JoyControlStickAxes class

    JoyControlStick::JoyStickDirections determineFourWayDiagonalDirection(); // JoyControlStickAxes class
    JoyControlStick::JoyStickDirections determineFourWayDiagonalDirection(int axisXValue,
                                                                          int axisYValue); // JoyControlStickAxes class

    JoyControlStick::JoyStickDirections calculateStickDirection();                               // JoyControlStickAxes class
    JoyControlStick::JoyStickDirections calculateStickDirection(int axisXValue, int axisYValue); // JoyControlStickAxes class

    void performButtonPress(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets);
    void performButtonRelease(JoyControlStickButton *&eventbutton, bool ignoresets);

    void resetButtons();

    double calculateXDistanceFromDeadZone(bool interpolate = false); // JoyControlStickAxes class
    double calculateXDistanceFromDeadZone(int axisXValue, int axisYValue,
                                          bool interpolate = false); // JoyControlStickAxes class

    double calculateYDistanceFromDeadZone(bool interpolate = false); // JoyControlStickAxes class
    double calculateYDistanceFromDeadZone(int axisXValue, int axisYValue,
                                          bool interpolate = false); // JoyControlStickAxes class

    int calculateCircleXValue(int axisXValue, int axisYValue); // JoyControlStickAxes class
    int calculateCircleYValue(int axisXValue, int axisYValue); // JoyControlStickAxes class

    double calculateEightWayDiagonalDistanceFromDeadZone();                               // JoyControlStickAxes class
    double calculateEightWayDiagonalDistanceFromDeadZone(int axisXValue, int axisYValue); // JoyControlStickAxes class
    double calculateEightWayDiagonalDistance(int axisXValue, int axisYValue);             // JoyControlStickAxes class

    QHash<JoyStickDirections, JoyControlStickButton *> getApplicableButtons();
    void clearPendingAxisEvents(); // JoyControlStickEvent class

  signals:
    void moved(int xaxis, int yaxis);
    void active(int xaxis, int yaxis);
    void released(int axis, int yaxis);
    void deadZoneChanged(int value);
    void diagonalRangeChanged(int value);
    void maxZoneChanged(int value);
    void modifierZoneChanged(int value);
    void circleAdjustChange(double circle);
    void stickDelayChanged(int value);
    void stickNameChanged();
    void joyModeChanged();
    void propertyUpdated();

  public slots:
    void reset();
    void setDeadZone(int value);
    void setMaxZone(int value);
    void setModifierZone(int value);
    void setModifierZoneInverted(bool value);
    void setDiagonalRange(int value);
    void setStickName(QString tempName);
    void setButtonsSpringRelativeStatus(bool value);
    void setCircleAdjust(double circle); // JoyControlStickAxes class
    void setStickDelay(int value);
    void setButtonsEasingDuration(double value);
    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

  private slots:
    void stickDirectionChangeEvent(); // JoyControlStickEvent class

  private:
    int originset;
    int deadZone;
    int m_modifier_zone;
    bool m_modifier_zone_inverted;
    int diagonalRange;
    int maxZone;
    int index;
    int stickDelay; // unsigned int

    double circle;

    bool isActive;
    bool safezone;
    bool pendingStickEvent;

    QPointer<JoyAxis> axisX;
    QPointer<JoyAxis> axisY;

    JoyControlStickButton *activeButton1;
    JoyControlStickButton *activeButton2;
    JoyControlStickButton *activeButton3;

    JoyStickDirections currentDirection;
    JoyMode currentMode;

    QString stickName;
    QString defaultStickName;

    QTimer directionDelayTimer;

    QHash<JoyStickDirections, JoyControlStickButton *> buttons;
    JoyControlStickModifierButton *modifierButton;

    void populateStickBtns();
};

#endif // JOYCONTROLSTICK_H
