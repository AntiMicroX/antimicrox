#ifndef JOYBUTTONSLOTSLIST_H
#define JOYBUTTONSLOTSLIST_H

#include "joybuttonslot.h"

#include <QObject>



class JoyButtonSlotsList : public QObject
{
    Q_OBJECT

public:
    explicit JoyButtonSlotsList(QObject *parent = nullptr);

    bool containsSequence();
    bool containsDistanceSlots(); // JoyButtonSlots class *
    bool containsReleaseSlots(); // JoyButtonSlots class *
    bool hasActiveSlots(); // JoyButtonSlots class *

    QList<JoyButtonSlot*>* getAssignedSlots(); // JoyButtonSlots class *
    QList<JoyButtonSlot*> const& getActiveSlots(); // JoyButtonSlots class *

    virtual QString getSlotsSummary(); // JoyButtonSlots class *
    virtual QString getSlotsString(); // JoyButtonSlots class *

    bool insertAssignedSlot(JoyButtonSlot *newSlot, bool updateActiveString=true); // JoyButtonSlots class


signals:
    void slotsChanged(); // JoyButtonSlots class *


public slots:
    void removeAssignedSlot(int index); // JoyButtonSlots class *

    bool setAssignedSlot(int code, int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(int code,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(int code, int alias,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class

    bool setAssignedSlot(JoyButtonSlot *otherSlot, int index); // JoyButtonSlots class

    bool insertAssignedSlot(int code, int alias, int index,
                            JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard); // JoyButtonSlots class


protected:
    void clearAssignedSlots(bool signalEmit=true); // JoyButtonSlots class *
    void checkTurboCondition(JoyButtonSlot *slot); // JoyButtonSlots class *

    static JoyButtonSlot *lastActiveKey; // JoyButtonSlots class*


protected slots:


private slots:
    void releaseActiveSlots(); // JoyButtonSlots class *


private:

    void findJoySlotsEnd(QListIterator<JoyButtonSlot*> *slotiter); // JoyButtonSlots class *
    void countActiveSlots(int tempcode, int& references, JoyButtonSlot* slot, QHash<int, int>& activeSlotsHash, bool& changeRepeatState, bool activeSlotHashWindows = false); // JoyButtonSlots class *
    void releaseMoveSlots(QList<JoyButton::mouseCursorInfo>& cursorSpeeds, JoyButtonSlot *slot, QList<int>& indexesToRemove); // JoyButtonSlots class *

    QList<JoyButtonSlot*>& getAssignmentsLocal(); // JoyButtonSlots class *
    QList<JoyButtonSlot*>& getActiveSlotsLocal(); // JoyButtonSlots class *

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
        if (currentChangedSlot) currentSetChangeSlot = nullptr;

        isKeyPressed = isButtonPressed = false;
        quitEvent = true;
    }

    QList<JoyButtonSlot*> assignments;
    QList<JoyButtonSlot*> activeSlots;
    QListIterator<JoyButtonSlot*> *slotiter;
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


};

#endif // JOYBUTTONSLOTSLIST_H
