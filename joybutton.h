#ifndef JOYBUTTON_H
#define JOYBUTTON_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QListIterator>
#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttonslot.h"

class JoyButton : public QObject
{
    Q_OBJECT
public:
    explicit JoyButton(QObject *parent = 0);
    explicit JoyButton(int index, int originset, QObject *parent=0);
    ~JoyButton();

    enum SetChangeCondition {SetChangeDisabled=0, SetChangeOneWay, SetChangeTwoWay, SetChangeWhileHeld};
    void joyEvent (bool pressed, bool ignoresets=false);
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

    void setChangeSetSelection(int index);
    int getSetSelection();

    void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    SetChangeCondition getChangeSetCondition();

    bool getButtonState();
    int getOriginSet();

    void release();
    bool containsSequence();

    static const QString xmlName;

protected:
    // Used to denote whether the actual joypad button is pressed
    bool isButtonPressed;
    // Used to denote whether the virtual key is pressed
    bool isKeyPressed;
    bool toggle;

    bool eventReset;
    bool quitEvent;
    // Used to denote the SDL index of the actual joypad button
    int index;
    int turboInterval;
    QTimer timer;
    bool isDown;
    bool useTurbo;
    QList<JoyButtonSlot*> assignments;
    QList<JoyButtonSlot*> activeSlots;
    QString customName;
    int mouseSpeedX;
    int mouseSpeedY;

    int setSelection;
    SetChangeCondition setSelectionCondition;
    int originset;
    QListIterator<JoyButtonSlot*> *slotiter;
    JoyButtonSlot *currentPause;
    JoyButtonSlot *currentHold;
    bool ignoresets;
    QMutex buttonMutex;
    QTime buttonHold;
    QTime pauseHold;
    QTime inpauseHold;
    QQueue<bool> ignoreSetQueue;
    QQueue<bool> isButtonPressedQueue;

signals:
    void clicked (int index);
    void released (int index);
    void keyChanged(int keycode);
    void mouseChanged(int mousecode);
    void setChangeActivated(int index);
    void setAssignmentChanged(int current_button, int associated_set, int mode);
    void finishedPause();

public slots:
    void setTurboInterval (int interval);
    void setToggle (bool toggle);
    virtual void reset();
    virtual void reset(int index);

private slots:
    void turboEvent();
    virtual void mouseEvent(JoyButtonSlot *buttonslot);
    void createDeskEvent();
    void releaseDeskEvent();
    void waitForDeskEvent();
    void waitForReleaseDeskEvent();
    void pauseEvent();
    void holdEvent();
    void pauseWaitEvent();
    void checkForSetChange();
};


#endif // JOYBUTTON_H
