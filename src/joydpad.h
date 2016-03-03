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

#ifndef JOYDPAD_H
#define JOYDPAD_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttontypes/joydpadbutton.h"

class JoyDPad : public QObject
{
    Q_OBJECT
public:
    explicit JoyDPad(int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    ~JoyDPad();

    enum JoyMode {StandardMode=0, EightWayMode, FourWayCardinal, FourWayDiagonal};

    JoyDPadButton* getJoyButton(int index);
    QHash<int, JoyDPadButton*>* getJoyButtons();

    int getCurrentDirection();
    int getJoyNumber();
    int getIndex();
    int getRealJoyNumber();
    virtual QString getName(bool fullForceFormat=false, bool displayNames=false);

    void joyEvent(int value, bool ignoresets=false);
    void queuePendingEvent(int value, bool ignoresets=false);
    void activatePendingEvent();
    bool hasPendingEvent();
    void clearPendingEvent();

    void setJoyMode(JoyMode mode);
    JoyMode getJoyMode();

    void releaseButtonEvents();

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

    QString getDpadName();

    virtual bool isDefault();

    QHash<int, JoyDPadButton*>* getButtons();

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    virtual QString getXmlName();

    virtual void setDefaultDPadName(QString tempname);
    virtual QString getDefaultDPadName();

    SetJoystick* getParentSet();
    bool hasSlotsAssigned();

    bool isRelativeSpring();
    void copyAssignments(JoyDPad *destDPad);

    unsigned int getDPadDelay();
    double getButtonsEasingDuration();

    void setButtonsSpringDeadCircleMultiplier(int value);
    int getButtonsSpringDeadCircleMultiplier();

    void setButtonsExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyExtraAccelerationCurve getButtonsExtraAccelerationCurve();

    QHash<int, JoyDPadButton*> getDirectionButtons(JoyDPadButton::JoyDPadDirections direction);

    void setDirButtonsUpdateInitAccel(JoyDPadButton::JoyDPadDirections direction, bool state);
    void copyLastDistanceValues(JoyDPad *srcDPad);

    virtual void eventReset();

    static const QString xmlName;
    static const unsigned int DEFAULTDPADDELAY;

protected:
    void populateButtons();
    void createDeskEvent(bool ignoresets = false);
    QHash<int, JoyDPadButton*> getApplicableButtons();
    bool readMainConfig(QXmlStreamReader *xml);

    QHash<int, JoyDPadButton*> buttons;
    int index;
    JoyDPadButton::JoyDPadDirections prevDirection;
    JoyDPadButton::JoyDPadDirections pendingDirection;
    JoyDPadButton *activeDiagonalButton;
    int originset;
    JoyMode currentMode;
    QString dpadName;
    QString defaultDPadName;
    SetJoystick *parentSet;
    QTimer directionDelayTimer;
    unsigned int dpadDelay;

    bool pendingEvent;
    int pendingEventDirection;
    bool pendingIgnoreSets;

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
};

#endif // JOYDPAD_H
