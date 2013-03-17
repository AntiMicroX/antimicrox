#ifndef JOYAXIS_H
#define JOYAXIS_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joyaxisbutton.h"

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

    void setDeadZone(int value);
    int getDeadZone();

    void setMaxZoneValue(int value);
    int getMaxZoneValue();
    void setThrottle(int value);
    int getThrottle();
    int getCurrentThrottledValue();
    int getCurrentRawValue();
    int getCurrentThrottledMin();
    int getCurrentThrottledMax();
    int getCurrentThrottledDeadValue();

    double getDistanceFromDeadZone();

    double calculateNormalizedAxisPlacement();

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    static const int AXISMIN;
    static const int AXISMAX;
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;

    static const float JOYSPEED;

protected:
    void createDeskEvent(bool ignoresets = false);
    void adjustRange();

    int index;
    int deadZone;
    int maxZoneValue;
    bool isActive;

    JoyAxisButton *paxisbutton;
    JoyAxisButton *naxisbutton;

    bool eventActive;
    int currentThrottledValue;
    int currentRawValue;
    QTimer *timer;
    QTime interval;
    int throttle;
    float sumDist;
    int mouseOffset;
    int lastkey;
    JoyAxisButton *activeButton;
    int originset;

    int currentThrottledMin;
    int currentThrottledMax;
    int currentThrottleCenter;
    int currentThrottledDeadValue;

signals:
    void active(int value);
    void released(int value);
    void moved(int value);

public slots:
    void reset();
    void reset(int index);

private slots:
    
};

#endif // JOYAXIS_H
