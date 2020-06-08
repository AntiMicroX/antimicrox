/* antimicroX Gamepad to KB+M event mapper
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


#ifndef JOYDPAD_H
#define JOYDPAD_H


#include "joybuttontypes/joydpadbutton.h"


class JoyDPad : public QObject
{
    Q_OBJECT

public:
    explicit JoyDPad(int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    ~JoyDPad();

    enum JoyMode {StandardMode=0, EightWayMode, FourWayCardinal, FourWayDiagonal};

    JoyDPadButton* getJoyButton(int index_local);
    QHash<int, JoyDPadButton*>* getJoyButtons();

    int getCurrentDirection();
    int getJoyNumber();
    int getIndex();
    int getRealJoyNumber();
    virtual QString getName(bool fullForceFormat=false, bool displayNames=false);

    void joyEvent(int value, bool ignoresets=false); // JoyDPadEvent class
    void queuePendingEvent(int value, bool ignoresets=false); // JoyDPadEvent class
    void activatePendingEvent(); // JoyDPadEvent class
    bool hasPendingEvent(); // JoyDPadEvent class
    void clearPendingEvent(); // JoyDPadEvent class

    void setJoyMode(JoyMode mode);
    JoyMode getJoyMode();

    void releaseButtonEvents(); // JoyDPadEvent class

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

    const QString getDpadName();
    const QString getDefaultDpadName();

    virtual bool isDefault();

    QHash<int, JoyDPadButton*>* getButtons();

    virtual QString getXmlName(); // JoyDPadXml class

    virtual void setDefaultDPadName(QString tempname);
    virtual QString getDefaultDPadName();

    SetJoystick* getParentSet();
    bool hasSlotsAssigned();

    bool isRelativeSpring();
    void copyAssignments(JoyDPad *destDPad);

    int getDPadDelay();
    double getButtonsEasingDuration();

    void setButtonsSpringDeadCircleMultiplier(int value);
    int getButtonsSpringDeadCircleMultiplier();

    void setButtonsExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyExtraAccelerationCurve getButtonsExtraAccelerationCurve();

    QHash<int, JoyDPadButton*> getDirectionButtons(JoyDPadButton::JoyDPadDirections direction);

    void setDirButtonsUpdateInitAccel(JoyDPadButton::JoyDPadDirections direction, bool state);
    void copyLastDistanceValues(JoyDPad *srcDPad);

    virtual void eventReset(); // JoyDPadEvent class

signals:
    void active(int value);
    void released(int value);
    void dpadNameChanged();
    void dpadDelayChanged(int value);
    void joyModeChanged();
    void propertyUpdated();

public slots:
    void setDPadName(QString tempName);
    void setButtonsSpringRelativeStatus(bool value);
    void setDPadDelay(int value);
    void setButtonsEasingDuration(double value);
    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();

private slots:
    void dpadDirectionChangeEvent();

protected:
    void populateButtons();
    void createDeskEvent(bool ignoresets = false); // JoyDPadEvent class
    QHash<int, JoyDPadButton*> getApplicableButtons();

private:
    QHash<int, JoyDPadButton*> buttons;

    JoyDPadButton::JoyDPadDirections prevDirection;
    JoyDPadButton::JoyDPadDirections pendingDirection;
    JoyDPadButton *activeDiagonalButton;

    QString dpadName;
    QString defaultDPadName;

    SetJoystick *m_parentSet;
    QTimer directionDelayTimer;
    JoyMode currentMode;

    int m_index;
    int m_originset;
    int dpadDelay; // unsigned
    int pendingEventDirection;

    bool pendingEvent;
    bool pendingIgnoreSets;

};

#endif // JOYDPAD_H
