#ifndef JOYBUTTON_H
#define JOYBUTTON_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>
#include <QList>
#include <QListIterator>
#include <QHash>
#include <QQueue>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttonslot.h"
#include "springmousemoveinfo.h"
#include "joybuttonmousehelper.h"

class VDPad;
class SetJoystick;

class JoyButton : public QObject
{
    Q_OBJECT
public:
    explicit JoyButton(int index, int originset, SetJoystick *parentSet, QObject *parent=0);
    ~JoyButton();

    enum SetChangeCondition {SetChangeDisabled=0, SetChangeOneWay,
                             SetChangeTwoWay, SetChangeWhileHeld};

    enum JoyMouseMovementMode {MouseCursor=0, MouseSpring};
    enum JoyMouseCurve {EnhancedPrecisionCurve=0, LinearCurve, QuadraticCurve,
                        CubicCurve, QuadraticExtremeCurve, PowerCurve,
                        EasingQuadraticCurve, EasingCubicCurve};
    enum TurboMode {NormalTurbo=0, GradientTurbo, PulseTurbo};

    void joyEvent (bool pressed, bool ignoresets=false);
    int getJoyNumber ();
    virtual int getRealJoyNumber ();
    void setJoyNumber (int index);

    bool getToggleState();
    int getTurboInterval();
    bool isUsingTurbo();
    void setCustomName(QString name);
    QString getCustomName();
    bool setAssignedSlot(int code,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool setAssignedSlot(int code, unsigned int alias,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool setAssignedSlot(int code, unsigned int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool insertAssignedSlot(int code, unsigned int alias, int index,
                            JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    bool setAssignedSlot(JoyButtonSlot *otherSlot, int index);

    void removeAssignedSlot(int index);

    QList<JoyButtonSlot*> *getAssignedSlots();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getSlotsSummary();
    virtual QString getSlotsString();
    virtual QString getActiveZoneSummary();
    virtual QString getName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();

    int getMouseSpeedX();
    int getMouseSpeedY();

    int getWheelSpeedX();
    int getWheelSpeedY();

    void setChangeSetSelection(int index);
    int getSetSelection();

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive=false);
    SetChangeCondition getChangeSetCondition();

    bool getButtonState();
    int getOriginSet();

    bool containsSequence();
    bool containsDistanceSlots();
    bool containsReleaseSlots();

    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();

    virtual void setVDPad(VDPad *vdpad);
    void removeVDPad();
    bool isPartVDPad();
    VDPad* getVDPad();

    virtual bool isDefault();
    void setIgnoreEventState(bool ignore);
    bool getIgnoreEventState();

    void setMouseMode(JoyMouseMovementMode mousemode);
    JoyMouseMovementMode getMouseMode();

    void setMouseCurve(JoyMouseCurve selectedCurve);
    JoyMouseCurve getMouseCurve();

    int getSpringWidth();
    int getSpringHeight();

    double getSensitivity();

    void setSmoothing(bool enabled=false);
    bool isSmoothingEnabled();
    bool getWhileHeldStatus();
    void setWhileHeldStatus(bool status);

    QString getActionName();
    QString getButtonName();

    virtual void setDefaultButtonName(QString tempname);
    virtual QString getDefaultButtonName();

    SetJoystick* getParentSet();

    void setCycleResetTime(unsigned int interval);
    unsigned int getCycleResetTime();

    void setCycleResetStatus(bool enabled);
    bool isCycleResetActive();

    bool isRelativeSpring();
    void copyAssignments(JoyButton *destButton);

    virtual void setTurboMode(TurboMode mode);
    TurboMode getTurboMode();
    virtual bool isPartRealAxis();
    virtual bool isModifierButton();

    static int calculateFinalMouseSpeed(JoyMouseCurve curve, int value);
    double getEasingDuration();

    static void moveMouseCursor(int &movedX, int &movedY, int &movedElapsed);
    static void moveSpringMouse(int &movedX, int &movedY, bool &hasMoved);

    static JoyButtonMouseHelper* getMouseHelper();
    static QList<JoyButton*>* getPendingMouseButtons();
    static bool hasCursorEvents();
    static bool hasSpringEvents();

    static double getWeightModifier();
    static void setWeightModifier(double modifier);

    static int getMouseHistorySize();
    static void setMouseHistorySize(int size);

    static int getMouseRefreshRate();
    static void setMouseRefreshRate(int refresh);

    static const QString xmlName;

    // Define default values for many properties.
    static const int ENABLEDTURBODEFAULT;
    static const double SMOOTHINGFACTOR;
    static const double DEFAULTMOUSESPEEDMOD;
    static const unsigned int DEFAULTKEYREPEATDELAY;
    static const unsigned int DEFAULTKEYREPEATRATE;
    static const JoyMouseCurve DEFAULTMOUSECURVE;
    static const bool DEFAULTTOGGLE;
    static const int DEFAULTTURBOINTERVAL;
    static const bool DEFAULTUSETURBO;
    static const int DEFAULTMOUSESPEEDX;
    static const int DEFAULTMOUSESPEEDY;
    static const int DEFAULTSETSELECTION;
    static const SetChangeCondition DEFAULTSETCONDITION;
    static const JoyMouseMovementMode DEFAULTMOUSEMODE;
    static const int DEFAULTSPRINGWIDTH;
    static const int DEFAULTSPRINGHEIGHT;
    static const double DEFAULTSENSITIVITY;
    static const bool DEFAULTSMOOTHING;
    static const int DEFAULTWHEELX;
    static const int DEFAULTWHEELY;
    static const bool DEFAULTCYCLERESETACTIVE;
    static const int DEFAULTCYCLERESET;
    static const bool DEFAULTRELATIVESPRING;
    static const TurboMode DEFAULTTURBOMODE;
    static const double DEFAULTEASINGDURATION;
    static const double MINIMUMEASINGDURATION;

    static const int DEFAULTMOUSEHISTORYSIZE;
    static const double DEFAULTWEIGHTMODIFIER;

    static const int MAXIMUMMOUSEHISTORYSIZE;
    static const double MAXIMUMWEIGHTMODIFIER;

    static const int MAXIMUMMOUSEREFRESHRATE;
    static const int IDLEMOUSEREFRESHRATE;

    static QList<double> mouseHistoryX;
    static QList<double> mouseHistoryY;

    static double cursorRemainderX;
    static double cursorRemainderY;

protected:
    double getTotalSlotDistance(JoyButtonSlot *slot);
    bool distanceEvent();
    void clearAssignedSlots(bool signalEmit=true);
    void releaseSlotEvent();
    void findReleaseEventEnd();
    void findHoldEventEnd();
    bool checkForDelaySequence();
    void checkForPressedSetChange();
    bool insertAssignedSlot(JoyButtonSlot *newSlot);
    unsigned int getPreferredKeyPressTime();
    void checkTurboCondition(JoyButtonSlot *slot);

    virtual bool readButtonConfig(QXmlStreamReader *xml);

    typedef struct mouseCursorInfo
    {
        JoyButtonSlot *slot;
        double code;
    } mouseCursorInfo;

    // Used to denote whether the actual joypad button is pressed
    bool isButtonPressed;
    // Used to denote whether the virtual key is pressed
    bool isKeyPressed;
    bool toggle;
    bool quitEvent;
    // Used to denote the SDL index of the actual joypad button
    int index;
    int turboInterval;

    QTimer turboTimer;
    QTimer pauseTimer;
    QTimer holdTimer;
    QTimer pauseWaitTimer;
    QTimer createDeskTimer;
    QTimer releaseDeskTimer;
    QTimer mouseWheelVerticalEventTimer;
    QTimer mouseWheelHorizontalEventTimer;
    QTimer setChangeTimer;
    QTimer keyPressTimer;
    QTimer delayTimer;
    QTimer keyRepeatTimer;
    static QTimer staticMouseEventTimer;

    bool isDown;
    bool toggleActiveState;
    bool useTurbo;
    QList<JoyButtonSlot*> assignments;
    QList<JoyButtonSlot*> activeSlots;
    QString customName;

    int mouseSpeedX;
    int mouseSpeedY;
    int wheelSpeedX;
    int wheelSpeedY;

    int setSelection;
    SetChangeCondition setSelectionCondition;
    int originset;

    QListIterator<JoyButtonSlot*> *slotiter;
    JoyButtonSlot *currentPause;
    JoyButtonSlot *currentHold;
    JoyButtonSlot *currentCycle;
    JoyButtonSlot *previousCycle;
    JoyButtonSlot *currentDistance;
    JoyButtonSlot *currentMouseEvent;
    JoyButtonSlot *currentRelease;
    JoyButtonSlot *currentWheelVerticalEvent;
    JoyButtonSlot *currentWheelHorizontalEvent;
    JoyButtonSlot *currentKeyPress;
    JoyButtonSlot *currentDelay;

    bool ignoresets;
    QTime buttonHold;
    QTime pauseHold;
    QTime inpauseHold;
    QTime buttonHeldRelease;
    QTime keyPressHold;
    QTime buttonDelay;
    QTime turboHold;
    QTime wheelVerticalTime;
    QTime wheelHorizontalTime;
    static QTime lastMouseTime;

    QQueue<bool> ignoreSetQueue;
    QQueue<bool> isButtonPressedQueue;

    QQueue<JoyButtonSlot*> mouseEventQueue;
    QQueue<JoyButtonSlot*> mouseWheelVerticalEventQueue;
    QQueue<JoyButtonSlot*> mouseWheelHorizontalEventQueue;

    int currentRawValue;
    VDPad *vdpad;
    bool ignoreEvents;
    JoyMouseMovementMode mouseMode;
    JoyMouseCurve mouseCurve;

    int springWidth;
    int springHeight;
    double sensitivity;
    bool smoothing;
    bool whileHeldStatus;
    double lastDistance;
    double lastWheelVerticalDistance;
    double lastWheelHorizontalDistance;
    int tempTurboInterval;

    QString actionName;
    QString buttonName; // User specified button name
    QString defaultButtonName; // Name used by the system

    SetJoystick *parentSet; // Pointer to set that button is assigned to.

    bool cycleResetActive;
    unsigned int cycleResetInterval;
    QTime cycleResetHold;

    bool relativeSpring;
    TurboMode currentTurboMode;

    double easingDuration;

    static double mouseSpeedModifier;
    static QList<JoyButtonSlot*> mouseSpeedModList;

    static QList<mouseCursorInfo> cursorXSpeeds;
    static QList<mouseCursorInfo> cursorYSpeeds;

    static QList<PadderCommon::springModeInfo> springXSpeeds;
    static QList<PadderCommon::springModeInfo> springYSpeeds;

    static QList<JoyButton*> pendingMouseButtons;

    static QHash<unsigned int, int> activeKeys;
    static QHash<unsigned int, int> activeMouseButtons;
    static JoyButtonSlot *lastActiveKey;
    static JoyButtonMouseHelper mouseHelper;
    static double weightModifier;
    static int mouseHistorySize;
    static int mouseRefreshRate;

signals:
    void clicked (int index);
    void released (int index);
    void keyChanged(int keycode);
    void mouseChanged(int mousecode);
    void setChangeActivated(int index);
    void setAssignmentChanged(int current_button, int associated_set, int mode);
    void finishedPause();
    void turboChanged(bool state);
    void toggleChanged(bool state);
    void turboIntervalChanged(int interval);
    void slotsChanged();
    void actionNameChanged();
    void buttonNameChanged();
    void propertyUpdated();
    void activeZoneChanged();

public slots:
    void setTurboInterval (int interval);
    void setToggle (bool toggle);
    void setUseTurbo(bool useTurbo);
    void setMouseSpeedX(int speed);
    void setMouseSpeedY(int speed);

    void setWheelSpeedX(int speed);
    void setWheelSpeedY(int speed);

    void setSpringWidth(int value);
    void setSpringHeight(int value);

    void setSensitivity(double value);
    void setSpringRelativeStatus(bool value);

    void setActionName(QString tempName);
    void setButtonName(QString tempName);

    void setEasingDuration(double value);

    virtual void reset();
    virtual void reset(int index);
    virtual void resetProperties();

    virtual void clearSlotsEventReset(bool clearSignalEmit=true);
    virtual void eventReset();

    void establishMouseTimerConnections();
    void establishPropertyUpdatedConnections();
    void disconnectPropertyUpdatedConnections();

    virtual void mouseEvent();

protected slots:
    virtual void turboEvent();
    void createDeskEvent();
    void releaseDeskEvent(bool skipsetchange=false);

private slots:
    void releaseActiveSlots();
    void activateSlots();
    void waitForDeskEvent();
    void waitForReleaseDeskEvent();
    void holdEvent();
    void delayEvent();
    void repeatKeysEvent();

    void wheelEventVertical();
    void wheelEventHorizontal();

    void pauseWaitEvent();
    void checkForSetChange();
    void keyPressEvent();
};


#endif // JOYBUTTON_H
