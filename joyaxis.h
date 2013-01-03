#ifndef JOYAXIS_H
#define JOYAXIS_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class JoyAxis : public QObject
{
    Q_OBJECT
public:
    explicit JoyAxis(QObject *parent = 0);
    explicit JoyAxis(int index, QObject *parent=0);
    ~JoyAxis();

    void joyEvent(int value);
    bool inDeadZone(int value);
    QString getName();
    void setIndex(int index);
    int getIndex();
    int getRealJoyIndex();
    void setPKey(int code);
    int getPKey();
    void setNKey(int code);
    int getNKey();
    void setDeadZone(int value);
    int getDeadZone();
    void setMouseMode(int mode);
    int getMouseMode();
    void setMouseSpeed(int speed);
    int getMouseSpeed();
    void setAxisMode(int mode);
    int getAxisMode();
    void setMaxZoneValue(int value);
    int getMaxZoneValue();
    void setThrottle(int value);
    int getThrottle();
    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    static const int AXISMIN;
    static const int AXISMAX;
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;
    static const int JOYINTERVAL;
    static const float JOYSPEED;

    enum JoyAxisMode {KeyboardAxis, MouseAxis};
    enum JoyAxisMouseMode {MouseHorizontal, MouseInvHorizontal, MouseVertical, MouseInvVertical};

protected:
    void createDeskEvent();

    int index;
    int deadZone;
    int maxZoneValue;
    bool isActive;
    int pkeycode;
    int nkeycode;
    int mouseSpeed;

    JoyAxisMode axisMode;
    JoyAxisMouseMode mousemode;
    bool trigger;
    bool eventActive;
    int currentValue;
    QTimer *timer;
    QTime interval;
    int throttle;
    float sumDist;
    int mouseOffset;
    int lastkey;

signals:
    void active(int value);
    void released(int value);
    void moved(int value);

public slots:
    void reset();
    void reset(int index);

private slots:
    void timerEvent();
    
};

#endif // JOYAXIS_H
