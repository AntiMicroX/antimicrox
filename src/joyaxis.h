#ifndef JOYAXIS_H
#define JOYAXIS_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joyaxisbutton.h"

class JoyControlStick;

class JoyAxis : public QObject
{
    Q_OBJECT
public:
    explicit JoyAxis(QObject *parent = 0);
    explicit JoyAxis(int index, int originset, QObject *parent=0);
    ~JoyAxis();

    void joyEvent(int value, bool ignoresets=false);
    bool inDeadZone(int value);
    QString getName();
    void setIndex(int index);
    int getIndex();
    int getRealJoyIndex();

    JoyAxisButton *getPAxisButton();
    JoyAxisButton *getNAxisButton();

    int getDeadZone();

    int getMaxZoneValue();
    void setThrottle(int value);
    int getThrottle();
    int getCurrentThrottledValue();
    int getCurrentRawValue();
    //int getCurrentThrottledMin();
    //int getCurrentThrottledMax();
    int getCurrentThrottledDeadValue();
    int getCurrentlyAssignedSet();

    double getDistanceFromDeadZone();
    double getSpringDistanceFromDeadZone();
    double calculateNormalizedAxisPlacement();
    double getAbsoluteAxisPlacement();

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    void setControlStick(JoyControlStick *stick);
    void removeControlStick();
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

    virtual bool isDefault();

    static const int AXISMIN;
    static const int AXISMAX;
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;

    static const float JOYSPEED;

protected:
    void createDeskEvent(bool ignoresets = false);
    void adjustRange();
    int calculateThrottledValue(int value);
    void setCurrentRawValue(int value);

    int index;
    int deadZone;
    int maxZoneValue;
    bool isActive;

    JoyAxisButton *paxisbutton;
    JoyAxisButton *naxisbutton;

    bool eventActive;
    int currentThrottledValue;
    int currentRawValue;
    //QTimer *timer;
    //QTime interval;
    int throttle;
    //float sumDist;
    JoyAxisButton *activeButton;
    int originset;

    //int currentThrottledMin;
    //int currentThrottledMax;
    int currentThrottledDeadValue;
    JoyControlStick *stick;

signals:
    void active(int value);
    void released(int value);
    void moved(int value);
    void throttleChangePropogated(int index);
    void throttleChanged();

public slots:
    void reset();
    void reset(int index);
    void propogateThrottleChange();    

    void setDeadZone(int value);
    void setMaxZoneValue(int value);
};

#endif // JOYAXIS_H
