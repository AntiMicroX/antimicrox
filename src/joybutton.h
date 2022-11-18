/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "globalvariables.h"
#include "joybuttonmousehelper.h"
#include "joybuttonslot.h"
#include "springmousemoveinfo.h"

#include <QDeadlineTimer>
#include <QQueue>
#include <QReadWriteLock>
#include <QRunnable>
#include <QThread>
#include <QTimer>

class VDPad;
class SetJoystick;
class QXmlStreamReader;
class QXmlStreamWriter;
// class QThread;
class QThreadPool;

/**
 * @brief Represents a single joystick button being part of a SetJoystick
 *  Contains multiple JoyButtonSlots which do the actual mapping.
 *  Also has various static methods for mouse cursor movement.
 */
class JoyButton : public QObject
{
    Q_OBJECT

  public:
    explicit JoyButton(int sdl_button_index, int originset, SetJoystick *parentSet, QObject *parent);
    ~JoyButton();

    enum SetChangeCondition
    {
        SetChangeDisabled = 0,
        SetChangeOneWay,
        SetChangeTwoWay,
        SetChangeWhileHeld
    };

    enum JoyMouseMovementMode
    {
        MouseCursor = 0,
        MouseSpring
    };
    enum JoyMouseCurve
    {
        EnhancedPrecisionCurve = 0,
        LinearCurve,
        QuadraticCurve,
        CubicCurve,
        QuadraticExtremeCurve,
        PowerCurve,
        EasingQuadraticCurve,
        EasingCubicCurve
    };
    enum JoyExtraAccelerationCurve
    {
        LinearAccelCurve,
        EaseOutSineCurve,
        EaseOutQuadAccelCurve,
        EaseOutCubicAccelCurve
    };
    enum TurboMode
    {
        NormalTurbo = 0,
        GradientTurbo,
        PulseTurbo
    };

    typedef struct _mouseCursorInfo
    {
        JoyButtonSlot *slot;
        double code;
    } mouseCursorInfo;

    void joyEvent(bool pressed, bool ignoresets = false);          // JoyButtonEvents class
    void queuePendingEvent(bool pressed, bool ignoresets = false); // JoyButtonEvents class
    void activatePendingEvent();                                   // JoyButtonEvents class
    void setJoyNumber(int index);
    void clearPendingEvent(); // JoyButtonEvents class
    void setCustomName(QString name);
    void setUpdateInitAccel(bool state);
    void removeVDPad();
    void setIgnoreEventState(bool ignore); // JoyButtonEvents class
    void setMouseMode(JoyMouseMovementMode mousemode);
    void setMouseCurve(JoyMouseCurve selectedCurve);
    void setWhileHeldStatus(bool status);
    void setCycleResetStatus(bool enabled);
    void copyAssignments(JoyButton *destButton);
    void resetAccelerationDistances();
    void setExtraAccelerationStatus(bool status);
    void setExtraAccelerationMultiplier(double value);
    void setCycleResetTime(int interval); // .., unsigned
    void setMinAccelThreshold(double value);
    void setExtraAccelerationCurve(JoyExtraAccelerationCurve curve);
    void setSpringDeadCircleMultiplier(int value);
    void setAccelExtraDuration(double value);
    void setStartAccelMultiplier(double value);
    void setMaxAccelThreshold(double value);
    void setChangeSetSelection(int index, bool updateActiveString = true);
    void activateMiniSlots(JoyButtonSlot *slot, JoyButtonSlot *mix);

    bool hasPendingEvent(); // JoyButtonEvents class
    bool getToggleState();
    bool isUsingTurbo();
    bool getButtonState();
    bool containsSequence();
    bool containsDistanceSlots(); // JoyButtonSlots class
    bool containsReleaseSlots();  // JoyButtonSlots class
    bool getIgnoreEventState();   // JoyButtonEvents class
    bool getWhileHeldStatus();
    bool hasActiveSlots(); // JoyButtonSlots class
    bool isCycleResetActive();
    bool isRelativeSpring();
    bool isPartVDPad();
    bool isExtraAccelerationEnabled();

    double getMinAccelThreshold();
    double getMaxAccelThreshold();
    double getStartAccelMultiplier();
    double getAccelExtraDuration();
    double getExtraAccelerationMultiplier();
    double getSensitivity();
    double getEasingDuration();

    int getJoyNumber();
    int getTurboInterval();
    int getMouseSpeedX();
    int getMouseSpeedY();
    int getWheelSpeedX();
    int getWheelSpeedY();
    int getSetSelection();
    int getOriginSet();
    int getSpringWidth();
    int getSpringHeight();
    int getCycleResetTime(); // unsigned
    int getSpringDeadCircleMultiplier();

    QString getCustomName();
    QString getActionName();
    QString getButtonName();

    QList<JoyButtonSlot *> *getAssignedSlots();     // JoyButtonSlots class
    QList<JoyButtonSlot *> const &getActiveSlots(); // JoyButtonSlots class

    virtual bool isPartRealAxis();
    virtual bool isModifierButton();
    bool isDefault();

    virtual JoyMouseCurve getDefaultMouseCurve() const;

    virtual int getRealJoyNumber() const;

    virtual double getDistanceFromDeadZone();
    virtual double getMouseDistanceFromDeadZone();
    virtual double getLastMouseDistanceFromDeadZone();
    virtual double getAccelerationDistance();
    virtual double getLastAccelerationDistance();

    virtual void initializeDistanceValues();
    virtual void setTurboMode(TurboMode mode);
    virtual void setDefaultButtonName(QString tempname);
    virtual void copyLastMouseDistanceFromDeadZone(
        JoyButton *srcButton); // Don't use direct assignment but copying from a current button.
    virtual void copyLastAccelerationDistance(JoyButton *srcButton);
    virtual void setVDPad(VDPad *vdpad);
    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive = false, bool updateActiveString = true);

    virtual QString getPartialName(bool forceFullFormat = false, bool displayNames = false) const;
    virtual QString getSlotsSummary(); // JoyButtonSlots class
    virtual QString getSlotsString();  // JoyButtonSlots class
    virtual QString getActiveZoneSummary();
    virtual QString getCalculatedActiveZoneSummary();
    virtual QString getName(bool forceFullFormat = false, bool displayNames = false);
    virtual QString getXmlName(); // JoyButtonXml class
    virtual QString getDefaultButtonName();

    virtual QList<JoyButtonSlot *> getActiveZoneList();

    SetChangeCondition getChangeSetCondition();

    VDPad *getVDPad();

    JoyMouseMovementMode getMouseMode();

    JoyMouseCurve getMouseCurve();

    SetJoystick *getParentSet();

    TurboMode getTurboMode();

    static int calculateFinalMouseSpeed(JoyMouseCurve curve, int value, const float joyspeed);

    static bool hasCursorEvents(QList<JoyButton::mouseCursorInfo> *cursorXSpeedsList,
                                QList<JoyButton::mouseCursorInfo> *cursorYSpeedsList); // JoyButtonEvents class
    static bool hasSpringEvents(QList<PadderCommon::springModeInfo> *springXSpeedsList,
                                QList<PadderCommon::springModeInfo> *springYSpeedsList); // JoyButtonEvents class
    static bool shouldInvokeMouseEvents(QList<JoyButton *> *pendingMouseButtons, QTimer *staticMouseEventTimer,
                                        QElapsedTimer *testOldMouseTime);

    static void setWeightModifier(double modifier, double maxWeightModifier, double &weightModifier);
    static void moveMouseCursor(int &movedX, int &movedY, int &movedElapsed, QList<double> *mouseHistoryX,
                                QList<double> *mouseHistoryY, QElapsedTimer *testOldMouseTime, QTimer *staticMouseEventTimer,
                                int mouseRefreshRate, int mouseHistorySize, QList<JoyButton::mouseCursorInfo> *cursorXSpeeds,
                                QList<JoyButton::mouseCursorInfo> *cursorYSpeeds, double &cursorRemainderX,
                                double &cursorRemainderY, double weightModifier, int idleMouseRefrRate,
                                QList<JoyButton *> *pendingMouseButtonse);
    static void moveSpringMouse(int &movedX, int &movedY, bool &hasMoved, int springModeScreen,
                                QList<PadderCommon::springModeInfo> *springXSpeeds,
                                QList<PadderCommon::springModeInfo> *springYSpeeds, QList<JoyButton *> *pendingMouseButtons,
                                int mouseRefreshRate, int idleMouseRefrRate, QTimer *staticMouseEventTimer);
    static void setMouseHistorySize(int size, int maxMouseHistSize, int &mouseHistSize, QList<double> *mouseHistoryX,
                                    QList<double> *mouseHistoryY);
    static void setMouseRefreshRate(int refresh, int &mouseRefreshRate, int idleMouseRefrRate,
                                    JoyButtonMouseHelper *mouseHelper, QList<double> *mouseHistoryX,
                                    QList<double> *mouseHistoryY, QElapsedTimer *testOldMouseTime,
                                    QTimer *staticMouseEventTimer);
    static void setSpringModeScreen(int screen, int &springModeScreen);
    static void resetActiveButtonMouseDistances(JoyButtonMouseHelper *mouseHelper);
    static void setGamepadRefreshRate(int refresh, int &gamepadRefreshRate, JoyButtonMouseHelper *mouseHelper);
    static void restartLastMouseTime(QElapsedTimer *testOldMouseTime);
    static void setStaticMouseThread(QThread *thread, QTimer *staticMouseEventTimer, QElapsedTimer *testOldMouseTime,
                                     int idleMouseRefrRate, JoyButtonMouseHelper *mouseHelper);
    static void indirectStaticMouseThread(QThread *thread, QTimer *staticMouseEventTimer, JoyButtonMouseHelper *mouseHelper);
    static void invokeMouseEvents(JoyButtonMouseHelper *mouseHelper); // JoyButtonEvents class

    static JoyButtonMouseHelper *getMouseHelper();
    static QList<JoyButton *> *getPendingMouseButtons();
    static QList<JoyButton::mouseCursorInfo> *getCursorXSpeeds();
    static QList<JoyButton::mouseCursorInfo> *getCursorYSpeeds();
    static QList<PadderCommon::springModeInfo> *getSpringXSpeeds();
    static QList<PadderCommon::springModeInfo> *getSpringYSpeeds();
    static QTimer *getStaticMouseEventTimer(); // JoyButtonEvents class
    static QElapsedTimer *getTestOldMouseTime();

    JoyExtraAccelerationCurve getExtraAccelerationCurve();

    static const JoyMouseCurve DEFAULTMOUSECURVE;
    static const SetChangeCondition DEFAULTSETCONDITION;
    static const JoyMouseMovementMode DEFAULTMOUSEMODE;
    static const TurboMode DEFAULTTURBOMODE;
    static const JoyExtraAccelerationCurve DEFAULTEXTRAACCELCURVE;

    bool insertAssignedSlot(JoyButtonSlot *slot, bool updateActiveString = true); // JoyButtonSlots class
    bool insertAssignedSlot(JoyButtonSlot *newSlot, int index, bool updateActiveString = true);
    bool containsJoyMixSlot();

  protected:
    int getPreferredKeyPressTime(); // unsigned

    double getTotalSlotDistance(JoyButtonSlot *slot);

    bool distanceEvent(); // JoyButtonEvents class
    bool checkForDelaySequence();
    void clearAssignedSlots(bool signalEmit = true); // JoyButtonSlots class
    void releaseSlotEvent();                         // JoyButtonEvents class
    void checkForPressedSetChange();
    void checkTurboCondition(JoyButtonSlot *slot);
    void vdpadPassEvent(bool pressed, bool ignoresets = false); // JoyButtonEvents class
    void localBuildActiveZoneSummaryString();

    static bool hasFutureSpringEvents(QList<JoyButton *> *pendingMouseButtons);
    static int timeBetweenMiniSlots;
    static int allSlotTimeBetweenSlots;

    virtual double getCurrentSpringDeadCircle();

    TurboMode currentTurboMode;

    QString buildActiveZoneSummary(QList<JoyButtonSlot *> &tempList);

    static QList<JoyButtonSlot *> mouseSpeedModList; // JoyButtonSlots class
    static QList<mouseCursorInfo> cursorXSpeeds;
    static QList<mouseCursorInfo> cursorYSpeeds;
    static QList<PadderCommon::springModeInfo> springXSpeeds;
    static QList<PadderCommon::springModeInfo> springYSpeeds;
    static QList<JoyButton *> pendingMouseButtons;
    static JoyButtonSlot *lastActiveKey; // JoyButtonSlots class
    static JoyButtonMouseHelper mouseHelper;

    int m_index_sdl; // Used to denote the SDL index of the actual joypad button
    int turboInterval;
    int wheelSpeedX;
    int wheelSpeedY;
    int setSelection;
    int tempTurboInterval;
    int springDeadCircleMultiplier;

    bool isButtonPressed; // Used to denote whether the actual joypad button is pressed
    bool isKeyPressed;    // Used to denote whether the virtual key is pressed

    double lastDistance;
    double lastWheelVerticalDistance;
    double lastWheelHorizontalDistance;

    QTimer turboTimer;
    QTimer mouseWheelVerticalEventTimer;
    QTimer mouseWheelHorizontalEventTimer;

    QElapsedTimer wheelVerticalTime;
    QElapsedTimer wheelHorizontalTime;
    QElapsedTimer turboHold;

    QPointer<SetJoystick> m_parentSet;
    SetChangeCondition setSelectionCondition;
    JoyButtonSlot *currentWheelVerticalEvent;   // JoyButtonEvents class
    JoyButtonSlot *currentWheelHorizontalEvent; // JoyButtonEvents class

    QQueue<bool> ignoreSetQueue;
    QQueue<bool> isButtonPressedQueue;
    QQueue<JoyButtonSlot *> mouseWheelVerticalEventQueue;   // JoyButtonEvents class
    QQueue<JoyButtonSlot *> mouseWheelHorizontalEventQueue; // JoyButtonEvents class

    QString buttonName;        // User specified button name
    QString defaultButtonName; // Name used by the system

  signals:
    void clicked(int index);
    void released(int index);
    void keyChanged(int keycode);
    void mouseChanged(int mousecode);
    void setChangeActivated(int index);
    void setAssignmentChanged(int current_button, int associated_set, int mode);
    void finishedPause();
    void turboChanged(bool state);
    void toggleChanged(bool state);
    void turboIntervalChanged(int interval);
    void slotsChanged(); // JoyButtonSlots class
    void actionNameChanged();
    void buttonNameChanged();
    void propertyUpdated();
    void activeZoneChanged();

  public slots:
    void setTurboInterval(int interval);
    void setToggle(bool toggle);
    void setUseTurbo(bool useTurbo);
    void setMouseSpeedX(int speed);
    void setMouseSpeedY(int speed);
    void setWheelSpeed(int speed, QChar ax);
    void setSpringWidth(int value);
    void setSpringHeight(int value);
    void setSensitivity(double value);
    void setSpringRelativeStatus(bool value);
    void setActionName(QString tempName);
    void setButtonName(QString tempName);
    void setEasingDuration(double value);
    void establishPropertyUpdatedConnections();
    void disconnectPropertyUpdatedConnections();
    void removeAssignedSlot(int index);

    virtual void reset();
    virtual void reset(int index);
    virtual void resetProperties();
    virtual void clearSlotsEventReset(bool clearSignalEmit = true); // JoyButtonEvents class
    virtual void eventReset();                                      // JoyButtonEvents class
    virtual void mouseEvent();                                      // JoyButtonEvents class

    static void establishMouseTimerConnections();

    bool setAssignedSlot(int code, int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode = JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(int code,
                         JoyButtonSlot::JoySlotInputAction mode = JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(int code, int alias,
                         JoyButtonSlot::JoySlotInputAction mode = JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(JoyButtonSlot *otherSlot, int index); // JoyButtonSlots class

    bool insertAssignedSlot(int code, int alias, int index,
                            JoyButtonSlot::JoySlotInputAction mode = JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    void buildActiveZoneSummaryString();

  protected slots:
    virtual void turboEvent();           // JoyButtonEvents class
    virtual void wheelEventVertical();   // JoyButtonEvents class
    virtual void wheelEventHorizontal(); // JoyButtonEvents class

    void createDeskEvent();                            // JoyButtonEvents class
    void releaseDeskEvent(bool skipsetchange = false); // JoyButtonEvents class

  private slots:
    void releaseActiveSlots();      // JoyButtonSlots class
    void activateSlots();           // JoyButtonSlots class
    void waitForDeskEvent();        // JoyButtonEvents class
    void waitForReleaseDeskEvent(); // JoyButtonEvents class
    void holdEvent();               // JoyButtonEvents class
    void delayEvent();              // JoyButtonEvents class
    void pauseWaitEvent();          // JoyButtonEvents class
    void checkForSetChange();
    void keyPressEvent(); // JoyButtonEvents class
    void slotSetChange();

  private:
    inline void updatePendingParams(bool isEvent, bool isPressed, bool areIgnoredSets)
    {
        pendingEvent = isEvent;
        pendingPress = isPressed;
        pendingIgnoreSets = areIgnoredSets;
    }

    inline void updateMouseParams(bool updatedLastDist, bool updatedStartDist, double oldAccelMultiVal)
    {
        updateLastMouseDistance = updatedLastDist;
        updateStartingMouseDistance = updatedStartDist;
        updateOldAccelMulti = oldAccelMultiVal;
    }

    inline void resetSlotsProp(bool currentChangedSlot = false) // JoyButtonSlots class
    {
        currentCycle = nullptr;
        previousCycle = nullptr;
        currentPause = nullptr;
        currentHold = nullptr;
        m_currentDistance = nullptr;
        currentRawValue = 0;
        currentMouseEvent = nullptr;
        currentRelease = nullptr;
        currentWheelVerticalEvent = nullptr;
        currentWheelHorizontalEvent = nullptr;
        currentKeyPress = nullptr;
        currentDelay = nullptr;
        if (currentChangedSlot)
            currentSetChangeSlot = nullptr;

        isKeyPressed = isButtonPressed = false;
        quitEvent = true;
    }

    inline void stopTimers(bool stoppedSlotSetTimer)
    {
        turboTimer.stop();
        pauseWaitTimer.stop();
        createDeskTimer.stop();
        releaseDeskTimer.stop();
        holdTimer.stop();
        mouseWheelVerticalEventTimer.stop();
        mouseWheelHorizontalEventTimer.stop();
        setChangeTimer.stop();
        keyPressTimer.stop();
        delayTimer.stop();

        if (stoppedSlotSetTimer)
            slotSetChangeTimer.stop();

        if (slotiter != nullptr)
        {
            delete slotiter;
            slotiter = nullptr;
        }
    }

    inline void clearQueues()
    {
        isButtonPressedQueue.clear();
        ignoreSetQueue.clear();
        mouseEventQueue.clear();
        mouseWheelVerticalEventQueue.clear();
        mouseWheelHorizontalEventQueue.clear();
    }

    inline void restartAccelParams(bool updatedOldAccel, bool restartedAccelDurTimer, bool restartedRestParams)
    {
        lastAccelerationDistance = 0.0;
        currentAccelMulti = 0.0;
        currentAccelerationDistance = 0.0;
        startingAccelerationDistance = 0.0;

        if (updatedOldAccel)
            oldAccelMulti = updateOldAccelMulti = 0.0;
        else
            oldAccelMulti = 0.0;

        accelTravel = 0.0;
        if (restartedAccelDurTimer)
            accelExtraDurationTime.restart();

        if (restartedRestParams)
        {
            updateInitAccelValues = true;
            extraAccelerationEnabled = false;
            extraAccelerationMultiplier = GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE;
            minMouseDistanceAccelThreshold = GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD;
            maxMouseDistanceAccelThreshold = GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD;
            startAccelMultiplier = GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER;
            accelDuration = GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION;
            extraAccelCurve = DEFAULTEXTRAACCELCURVE;
        }
    }

    inline void lockForWritedString(QString &param, QString value)
    {
        activeZoneStringLock.lockForWrite();
        param = value;
        activeZoneStringLock.unlock();
    }

    void releaseEachSlot(bool &changeRepeatState, int &references, int tempcode, JoyButtonSlot::JoySlotInputAction mode,
                         JoyButtonSlot *slot);
    void resetAllProperties();
    void resetPrivVars();
    void restartAllForSetChange();
    void startTimerOverrun(int slotCode, QElapsedTimer *currSlotTime, QTimer *currSlotTimer, bool releasedDeskTimer = false);
    void findJoySlotsEnd(QListIterator<JoyButtonSlot *> *slotiter);
    void changeStatesQueue(bool currentReleased);
    void countActiveSlots(int tempcode, int &references, JoyButtonSlot *slot, QHash<int, int> &activeSlotsHash,
                          bool &changeRepeatState, bool activeSlotHashWindows = false); // JoyButtonSlots class
    void releaseMoveSlots(QList<JoyButton::mouseCursorInfo> &cursorSpeeds, JoyButtonSlot *slot,
                          QList<int> &indexesToRemove); // JoyButtonSlots class
    void setSpringDeadCircle(double &springDeadCircle, int mouseDirection);
    void checkSpringDeadCircle(int tempcode, double &springDeadCircle, int mouseSlot1, int mouseSlot2);
    static void distanceForMovingAx(double &finalAx, mouseCursorInfo infoAx);
    static void adjustAxForCursor(QList<double> *mouseHistoryList, double &adjustedAx, double &cursorRemainder,
                                  double weightModifier);
    void setDistanceForSpring(JoyButtonMouseHelper &mouseHelper, double &mouseFirstAx, double &mouseSecondAx,
                              double distanceFromDeadZone);
    void changeTurboParams(bool _isKeyPressed, bool isButtonPressed);
    void updateParamsAfterDistEvent(); // JoyButtonEvents class
    void startSequenceOfPressActive(bool isTurbo, QString debugText);
    QList<JoyButtonSlot *> &getAssignmentsLocal();
    QList<JoyButtonSlot *> &getActiveSlotsLocal(); // JoyButtonSlots class
    void updateMouseProperties(double newAxisValue, double newSpringDead, int newSpringWidth, int newSpringHeight,
                               bool relatived, int modeScreen, QList<PadderCommon::springModeInfo> &springSpeeds, QChar axis,
                               double newAxisValueY = 0, double newSpringDeadY = 0);
    // void getActiveZoneWithAppend(JoyButtonSlot::JoySlotInputAction mode, QList<JoyButtonSlot *>& tempSlotList,
    // QListIterator<JoyButtonSlot *> *iter, JoyButtonSlot *slot);
    void buildActiveZoneSummarySwitchSlots(JoyButtonSlot::JoySlotInputAction mode, JoyButtonSlot *slot, bool &behindHold,
                                           QStringList *stringlist, int &i, QListIterator<JoyButtonSlot *> *iter,
                                           bool slotsActive);

    bool m_toggle;
    bool quitEvent; // JoyButtonEvents class
    bool isDown;
    bool toggleActiveState;
    bool m_useTurbo;
    bool lastUnlessInList;
    bool m_ignoresets;
    bool ignoreEvents; // JoyButtonEvents class
    bool whileHeldStatus;
    bool updateLastMouseDistance;     // Should lastMouseDistance be updated. Set after mouse event.
    bool updateStartingMouseDistance; // Should startingMouseDistance be updated. Set after acceleration has finally been
                                      // applied.
    bool relativeSpring;
    bool pendingPress;
    bool pendingEvent; // JoyButtonEvents class
    bool pendingIgnoreSets;
    bool extraAccelerationEnabled;
    bool cycleResetActive;
    bool updateInitAccelValues;

    int mouseSpeedX;
    int mouseSpeedY;
    int m_originset;
    int springWidth;
    int springHeight;
    int currentRawValue;
    int cycleResetInterval; // unsigned

    double sensitivity;
    double lastMouseDistance;        // Keep track of the previous mouse distance from the previous gamepad poll.
    double lastAccelerationDistance; // Keep track of the previous full distance from the previous gamepad poll
    double currentAccelMulti;        // Multiplier and time used for acceleration easing.
    double accelDuration;
    double oldAccelMulti;
    double accelTravel; // Track travel when accel started
    double updateOldAccelMulti;
    double currentMouseDistance; // Keep track of the current mouse distance after a poll. Used to update lastMouseDistance
                                 // later.
    double currentAccelerationDistance;  // Keep track of the current mouse distance after a poll. Used to update
                                         // lastMouseDistance later.
    double startingAccelerationDistance; // Take into account when mouse acceleration started
    double minMouseDistanceAccelThreshold;
    double maxMouseDistanceAccelThreshold;
    double startAccelMultiplier;
    double m_easingDuration;
    double extraAccelerationMultiplier;

    QTimer pauseTimer;
    QTimer holdTimer;
    QTimer pauseWaitTimer;
    QTimer createDeskTimer;
    QTimer releaseDeskTimer;
    QTimer setChangeTimer;
    QTimer keyPressTimer;
    QTimer delayTimer;
    QTimer slotSetChangeTimer;
    static QTimer staticMouseEventTimer; // JoyButtonEvents class

    QString customName;
    QString actionName;
    QString activeZoneString;

    QList<JoyButtonSlot *> assignments;
    QList<JoyButtonSlot *> activeSlots;
    QListIterator<JoyButtonSlot *> *slotiter;
    QQueue<JoyButtonSlot *> mouseEventQueue; // JoyButtonEvents class
    JoyButtonSlot *currentPause;
    JoyButtonSlot *currentHold;
    JoyButtonSlot *currentCycle;
    JoyButtonSlot *previousCycle;
    JoyButtonSlot *m_currentDistance;
    JoyButtonSlot *currentMouseEvent; // JoyButtonEvents class
    JoyButtonSlot *currentRelease;
    JoyButtonSlot *currentKeyPress;
    JoyButtonSlot *currentDelay;
    JoyButtonSlot *currentSetChangeSlot;

    QElapsedTimer buttonHold;
    QElapsedTimer pauseHold;
    QElapsedTimer inpauseHold;
    QElapsedTimer buttonHeldRelease;
    QElapsedTimer keyPressHold;
    QElapsedTimer buttonDelay;
    QElapsedTimer accelExtraDurationTime;
    QElapsedTimer cycleResetHold;
    static QElapsedTimer testOldMouseTime;

    VDPad *m_vdpad;
    JoyMouseMovementMode mouseMode;
    JoyMouseCurve mouseCurve;
    JoyExtraAccelerationCurve extraAccelCurve;

    QReadWriteLock activeZoneLock;
    QReadWriteLock assignmentsLock;
    QReadWriteLock activeZoneStringLock;
    QThreadPool *threadPool;

    void addEachSlotToActives(JoyButtonSlot *slot, int &i, bool &delaySequence, bool &exit,
                              QListIterator<JoyButtonSlot *> *slotiter);
};

class MiniSlotRun : public QRunnable, public QObject
{
  public:
    MiniSlotRun(JoyButtonSlot *slot, JoyButtonSlot *slotmini, JoyButton *btn, int milisec)
        : QObject(btn)
        , m_slot(slot)
        , m_slotmini(slotmini)
        , m_btn(btn)
        , m_miliseconds(milisec)
    {
    }

    ~MiniSlotRun() {}

    void run()
    {
        this->thread()->wait(m_miliseconds);

        m_btn->activateMiniSlots(m_slotmini, m_slot);
    }

  private:
    JoyButtonSlot *m_slot;
    JoyButtonSlot *m_slotmini;
    JoyButton *m_btn;
    int m_miliseconds;
};

#endif // JOYBUTTON_H
