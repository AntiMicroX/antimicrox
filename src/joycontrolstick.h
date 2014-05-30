#ifndef JOYCONTROLSTICK_H
#define JOYCONTROLSTICK_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joyaxis.h"
#include "joybutton.h"
#include "joycontrolstickdirectionstype.h"
#include "joycontrolstickbutton.h"

class JoyControlStick : public QObject, public JoyStickDirectionsType
{
    Q_OBJECT
public:
    explicit JoyControlStick(JoyAxis *axisX, JoyAxis *axisY, int index, int originset = 0, QObject *parent = 0);
    ~JoyControlStick();

    enum JoyMode {StandardMode=0, EightWayMode, FourWayCardinal, FourWayDiagonal};

    void joyEvent(bool ignoresets=false);
    bool inDeadZone();
    int getDeadZone();
    int getDiagonalRange();
    double getDistanceFromDeadZone();
    double getAbsoluteDistance();
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
    double calculateBearing();
    QList<int> getDiagonalZoneAngles();
    QList<int> getFourWayCardinalZoneAngles();
    QList<int> getFourWayDiagonalZoneAngles();
    QHash<JoyStickDirections, JoyControlStickButton*>* getButtons();
    JoyAxis* getAxisX();
    JoyAxis* getAxisY();

    void replaceXAxis(JoyAxis *axis);
    void replaceYAxis(JoyAxis *axis);
    void replaceAxes(JoyAxis *axisX, JoyAxis* axisY);

    JoyControlStickButton* getDirectionButton(JoyStickDirections direction);
    double calculateNormalizedAxis1Placement();
    double calculateNormalizedAxis2Placement();
    double calculateDirectionalDistance(JoyControlStickButton *button, JoyButton::JoyMouseMovementMode=JoyButton::MouseCursor);

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

    void setButtonsSmoothing(bool enabled=false);
    bool getButtonsPresetSmoothing();

    void setButtonsWheelSpeedX(int value);
    void setButtonsWheelSpeedY(int value);

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

    static const double PI;

protected:
    virtual void populateButtons();
    void createDeskEvent(bool ignoresets = false);

    void determineStandardModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2);
    void determineEightWayModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2, JoyControlStickButton *&eventbutton3);
    void determineFourWayCardinalEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2);
    void determineFourWayDiagonalEvent(JoyControlStickButton *&eventbutton3);

    void performButtonPress(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets);
    void performButtonRelease(JoyControlStickButton *&eventbutton, bool ignoresets);

    void refreshButtons();
    void deleteButtons();
    void resetButtons();
    double calculateXDistanceFromDeadZone();
    double calculateYDistanceFromDeadZone();
    QHash<JoyStickDirections, JoyControlStickButton*> getApplicableButtons();

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

    QHash<JoyStickDirections, JoyControlStickButton*> buttons;

signals:
    void moved(int xaxis, int yaxis);
    void active(int xaxis, int yaxis);
    void released(int axis, int yaxis);
    void deadZoneChanged(int value);
    void diagonalRangeChanged(int value);
    void maxZoneChanged(int value);
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

    void establishPropertyUpdatedConnection();
    void disconnectPropertyUpdatedConnection();
};

#endif // JOYCONTROLSTICK_H
