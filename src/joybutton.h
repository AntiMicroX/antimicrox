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
#include <QReadWriteLock>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "joybuttonslot.h"
#include "springmousemoveinfo.h"
#include "joybuttonmousehelper.h"

#ifdef Q_OS_WIN
  #include "joykeyrepeathelper.h"
#endif

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
    enum JoyExtraAccelerationCurve {LinearAccelCurve, EaseOutSineCurve,
                                    EaseOutQuadAccelCurve, EaseOutCubicAccelCurve};
    enum TurboMode {NormalTurbo=0, GradientTurbo, PulseTurbo};

    void joyEvent(bool pressed, bool ignoresets=false);
    void queuePendingEvent(bool pressed, bool ignoresets=false);
    void activatePendingEvent();
    bool hasPendingEvent();
    void clearPendingEvent();

    int getJoyNumber();
    virtual int getRealJoyNumber();
    void setJoyNumber(int index);

    bool getToggleState();
    int getTurboInterval();
    bool isUsingTurbo();
    void setCustomName(QString name);
    QString getCustomName();

    QList<JoyButtonSlot*> *getAssignedSlots();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual QString getPartialName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getSlotsSummary();
    virtual QString getSlotsString();
    virtual QList<JoyButtonSlot*> getActiveZoneList();
    virtual QString getActiveZoneSummary();
    virtual QString getCalculatedActiveZoneSummary();
    virtual QString getName(bool forceFullFormat=false, bool displayNames=false);
    virtual QString getXmlName();

    int getMouseSpeedX();
    int getMouseSpeedY();

    int getWheelSpeedX();
    int getWheelSpeedY();

    void setChangeSetSelection(int index, bool updateActiveString=true);
    int getSetSelection();

    virtual void setChangeSetCondition(SetChangeCondition condition,
                                       bool passive=false, bool updateActiveString=true);
    SetChangeCondition getChangeSetCondition();

    bool getButtonState();
    int getOriginSet();

    bool containsSequence();
    bool containsDistanceSlots();
    bool containsReleaseSlots();

    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual double getLastMouseDistanceFromDeadZone();

    // Don't use direct assignment but copying from a current button.
    virtual void copyLastMouseDistanceFromDeadZone(JoyButton *srcButton);
    virtual void copyLastAccelerationDistance(JoyButton *srcButton);

    void copyExtraAccelerationState(JoyButton *srcButton);
    void setUpdateInitAccel(bool state);

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

    bool hasActiveSlots();

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

    static int getSpringModeScreen();
    static void setSpringModeScreen(int screen);

    static void resetActiveButtonMouseDistances();
    void resetAccelerationDistances();

    void setExtraAccelerationStatus(bool status);
    void setExtraAccelerationMultiplier(double value);

    bool isExtraAccelerationEnabled();
    double getExtraAccelerationMultiplier();

    virtual void initializeDistanceValues();

    void setMinAccelThreshold(double value);
    double getMinAccelThreshold();

    void setMaxAccelThreshold(double value);
    double getMaxAccelThreshold();

    void setStartAccelMultiplier(double value);
    double getStartAccelMultiplier();

    void setAccelExtraDuration(double value);
    double getAccelExtraDuration();

    void setExtraAccelerationCurve(JoyExtraAccelerationCurve curve);
    JoyExtraAccelerationCurve getExtraAccelerationCurve();

    virtual double getAccelerationDistance();
    virtual double getLastAccelerationDistance();

    void setSpringDeadCircleMultiplier(int value);
    int getSpringDeadCircleMultiplier();

    static int getGamepadRefreshRate();
    static void setGamepadRefreshRate(int refresh);

    static void restartLastMouseTime();

    static void setStaticMouseThread(QThread *thread);
    static void indirectStaticMouseThread(QThread *thread);

    static bool shouldInvokeMouseEvents();
    static void invokeMouseEvents();

    static const QString xmlName;

    // Define default values for many properties.
    static const int ENABLEDTURBODEFAULT;
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
    static const int DEFAULTWHEELX;
    static const int DEFAULTWHEELY;
    static const bool DEFAULTCYCLERESETACTIVE;
    static const int DEFAULTCYCLERESET;
    static const bool DEFAULTRELATIVESPRING;
    static const TurboMode DEFAULTTURBOMODE;
    static const double DEFAULTEASINGDURATION;
    static const double MINIMUMEASINGDURATION;
    static const double MAXIMUMEASINGDURATION;

    static const int DEFAULTMOUSEHISTORYSIZE;
    static const double DEFAULTWEIGHTMODIFIER;

    static const int MAXIMUMMOUSEHISTORYSIZE;
    static const double MAXIMUMWEIGHTMODIFIER;

    static const int MAXIMUMMOUSEREFRESHRATE;
    static const int DEFAULTIDLEMOUSEREFRESHRATE;
    static int IDLEMOUSEREFRESHRATE;

    static const unsigned int MINCYCLERESETTIME;
    static const unsigned int MAXCYCLERESETTIME;

    static const double DEFAULTEXTRACCELVALUE;
    static const double DEFAULTMINACCELTHRESHOLD;
    static const double DEFAULTMAXACCELTHRESHOLD;
    static const double DEFAULTSTARTACCELMULTIPLIER;
    static const double DEFAULTACCELEASINGDURATION;
    static const JoyExtraAccelerationCurve DEFAULTEXTRAACCELCURVE;

    static const int DEFAULTSPRINGRELEASERADIUS;

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
    void findReleaseEventIterEnd(QListIterator<JoyButtonSlot*> *tempiter);
    void findHoldEventEnd();
    bool checkForDelaySequence();
    void checkForPressedSetChange();
    bool insertAssignedSlot(JoyButtonSlot *newSlot, bool updateActiveString=true);
    unsigned int getPreferredKeyPressTime();
    void checkTurboCondition(JoyButtonSlot *slot);
    static bool hasFutureSpringEvents();
    virtual double getCurrentSpringDeadCircle();
    void vdpadPassEvent(bool pressed, bool ignoresets=false);

    QString buildActiveZoneSummary(QList<JoyButtonSlot*> &tempList);
    void localBuildActiveZoneSummaryString();

    virtual bool readButtonConfig(QXmlStreamReader *xml);

    typedef struct _mouseCursorInfo
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
    QTimer slotSetChangeTimer;
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
    JoyButtonSlot *currentSetChangeSlot;

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
    //static QElapsedTimer lastMouseTime;
    static QTime testOldMouseTime;

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

    // Keep track of the previous mouse distance from the previous gamepad
    // poll.
    double lastMouseDistance;

    // Keep track of the previous full distance from the previous gamepad
    // poll.
    double lastAccelerationDistance;

    // Multiplier and time used for acceleration easing.
    double currentAccelMulti;
    QTime accelExtraDurationTime;
    double accelDuration;
    double oldAccelMulti;
    double accelTravel; // Track travel when accel started

    // Should lastMouseDistance be updated. Set after mouse event.
    bool updateLastMouseDistance;

    // Should startingMouseDistance be updated. Set after acceleration
    // has finally been applied.
    bool updateStartingMouseDistance;

    double updateOldAccelMulti;

    bool updateInitAccelValues;

    // Keep track of the current mouse distance after a poll. Used
    // to update lastMouseDistance later.
    double currentMouseDistance;

    // Keep track of the current mouse distance after a poll. Used
    // to update lastMouseDistance later.
    double currentAccelerationDistance;

    // Take into account when mouse acceleration started
    double startingAccelerationDistance;

    double minMouseDistanceAccelThreshold;
    double maxMouseDistanceAccelThreshold;
    double startAccelMultiplier;

    JoyExtraAccelerationCurve extraAccelCurve;

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

    bool extraAccelerationEnabled;
    double extraAccelerationMultiplier;

    int springDeadCircleMultiplier;

    bool pendingPress;
    bool pendingEvent;
    bool pendingIgnoreSets;

    QReadWriteLock activeZoneLock;
    QReadWriteLock assignmentsLock;
    QReadWriteLock activeZoneStringLock;

    QString activeZoneString;
    QTimer activeZoneTimer;

    static double mouseSpeedModifier;
    static QList<JoyButtonSlot*> mouseSpeedModList;

    static QList<mouseCursorInfo> cursorXSpeeds;
    static QList<mouseCursorInfo> cursorYSpeeds;

    static QList<PadderCommon::springModeInfo> springXSpeeds;
    static QList<PadderCommon::springModeInfo> springYSpeeds;

    static QList<JoyButton*> pendingMouseButtons;

    static QHash<unsigned int, int> activeKeys;
    static QHash<unsigned int, int> activeMouseButtons;
#ifdef Q_OS_WIN
    static JoyKeyRepeatHelper repeatHelper;
#endif

    static JoyButtonSlot *lastActiveKey;
    static JoyButtonMouseHelper mouseHelper;
    static double weightModifier;
    static int mouseHistorySize;
    static int mouseRefreshRate;
    static int springModeScreen;
    static int gamepadRefreshRate;

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

    bool setAssignedSlot(int code, unsigned int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool setAssignedSlot(int code,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool setAssignedSlot(int code, unsigned int alias,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    bool setAssignedSlot(JoyButtonSlot *otherSlot, int index);

    bool insertAssignedSlot(int code, unsigned int alias, int index,
                            JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);

    void removeAssignedSlot(int index);

    static void establishMouseTimerConnections();
    void establishPropertyUpdatedConnections();
    void disconnectPropertyUpdatedConnections();

    virtual void mouseEvent();

protected slots:
    virtual void turboEvent();
    virtual void wheelEventVertical();
    virtual void wheelEventHorizontal();
    void createDeskEvent();
    void releaseDeskEvent(bool skipsetchange=false);
    void buildActiveZoneSummaryString();

private slots:
    void releaseActiveSlots();
    void activateSlots();
    void waitForDeskEvent();
    void waitForReleaseDeskEvent();
    void holdEvent();
    void delayEvent();

    void pauseWaitEvent();
    void checkForSetChange();
    void keyPressEvent();
    void slotSetChange();
};


#endif // JOYBUTTON_H
