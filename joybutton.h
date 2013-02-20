#ifndef JOYBUTTON_H
#define JOYBUTTON_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttonslot.h"

class JoyButton : public QObject
{
    Q_OBJECT
public:
    explicit JoyButton(QObject *parent = 0);
    explicit JoyButton(int index, QObject *parent=0);
    ~JoyButton();

    void joyEvent (bool pressed);
    int getJoyNumber ();
    virtual int getRealJoyNumber ();
    void setJoyNumber (int index);

    bool getToggleState();
    int getTurboInterval();
    void setUseTurbo(bool useTurbo);
    bool isUsingTurbo();
    void setCustomName(QString name);
    QString getCustomName();
    void setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setAssignedSlot(int code, int index, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    QList<JoyButtonSlot*> *getAssignedSlots();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual QString getPartialName();
    virtual QString getSlotsSummary();
    virtual QString getName();
    virtual QString getXmlName();

    void setMouseSpeedX(int speed);
    int getMouseSpeedX();
    void setMouseSpeedY(int speed);
    int getMouseSpeedY();

    static const QString xmlName;

protected:
    void createDeskEvent();

    // Used to denote whether the actual joypad button is pressed
    bool isButtonPressed;
    // Used to denote whether the virtual key is pressed
    bool isKeyPressed;
    bool toggle;
    // Used to denote the SDL index of the actual joypad button
    int index;
    int turboInterval;
    QTimer timer;
    bool isDown;
    bool useTurbo;
    QList<JoyButtonSlot*> assignments;
    QString customName;
    int mouseSpeedX;
    int mouseSpeedY;

signals:
    void clicked (int index);
    void released (int index);
    void keyChanged(int keycode);
    void mouseChanged(int mousecode);

public slots:
    void setTurboInterval (int interval);
    void setToggle (bool toggle);
    virtual void reset();
    virtual void reset(int index);

private slots:
    void turboEvent();
    virtual void mouseEvent(JoyButtonSlot *buttonslot);
};


#endif // JOYBUTTON_H
