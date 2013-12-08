#ifndef JOYBUTTONSLOT_H
#define JOYBUTTONSLOT_H

#include <QObject>
#include <QTime>
#include <QMetaType>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class JoyButtonSlot : public QObject
{
    Q_OBJECT
public:
    enum JoySlotInputAction {JoyKeyboard=0, JoyMouseButton, JoyMouseMovement, JoyPause, JoyHold, JoyCycle, JoyDistance, JoyRelease, JoyMouseSpeedMod};
    enum JoySlotMouseDirection {MouseUp=1, MouseDown, MouseLeft, MouseRight};
    enum JoySlotMouseWheelButton {MouseWheelUp=4, MouseWheelDown=5, MouseWheelLeft=6, MouseWheelRight=7};

    explicit JoyButtonSlot(QObject *parent = 0);
    explicit JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent=0);
    ~JoyButtonSlot();

    void setSlotCode(int code);
    int getSlotCode();
    void setSlotMode(JoySlotInputAction selectedMode);
    JoySlotInputAction getSlotMode();
    QString movementString();
    void setMouseSpeed(int value);
    void setDistance(double distance);
    double getMouseDistance();
    QTime* getMouseInterval();
    void restartMouseInterval();
    QString getXmlName();
    QString getSlotString();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int JOYSPEED;
    static const QString xmlName;
    
protected:
    int deviceCode;
    JoySlotInputAction mode;
    double distance;
    QTime *mouseInterval;

signals:
    
public slots:
    
};

Q_DECLARE_METATYPE(JoyButtonSlot*)


#endif // JOYBUTTONSLOT_H
