#include "advancebuttondialog.h"

#include <QtTest/QtTest>

/*
window title should be Advanced: gamepadBtn
first slot cannot be empty
first slot value should be equal to last pressed button
if getAssignedSlots() is not empty then slots should be equal and in order to getAssignedSlots() elements

if from comboBox is chosen "Blank or KB/M" then text should be tr("Insert a new blank slot.")

if from comboBox is chosen "Cycle" then text should be tr("Slots past a Cycle action will be executed "
                                          "on the next button press. Multiple cycles can be added "
                                          "in order to create partitions in a sequence.")

if from comboBox is chosen "Delay" then text should be tr("Delays the time that the next slot is activated "
                                          "by the time specified. Slots activated before the "
                                          "delay will remain active after the delay time "
                                          "has passed.")

if from comboBox is chosen "Distance" then text should be tr("Distance action specifies that the slots afterwards "
                                          "will only be executed when an axis is moved "
                                          "a certain range past the designated dead zone.")

if from comboBox is chosen "Execute" then text should be tr("Execute program when slot is activated.")

if from comboBox is chosen "Hold" then text should be tr("Insert a hold action. Slots after the action will only be "
                                          "executed if the button is held past the interval specified.")

if from comboBox is chosen "Load" then text should be tr("Chose a profile to load when this slot is activated.")

if from comboBox is chosen "Mouse Mod" then text should be tr("Mouse mod action will modify all mouse speed settings "
                                          "by a specified percentage while the action is being processed. "
                                          "This can be useful for slowing down the mouse while "
                                          "sniping.")

if from comboBox is chosen "Pause" then text should be tr("Insert a pause that occurs in between key presses.")

if from comboBox is chosen "Press Time" then text should be tr("Specify the time that keys past this slot should be "
                                          "held down.")

if from comboBox is chosen "Release" then text should be tr("Insert a release action. Slots after the action will only be "
                                          "executed after a button release if the button was held "
                                          "past the interval specified.")

if from comboBox is chosen "Set Change" then text should be tr("Change to selected set once slot is activated.")

if from comboBox is chosen "Text Entry" then text should be tr("Full string will be typed when a "
                                          "slot is activated.")

if last empty slot clicked and press some keyboard value it should hold new value from keyboard

if clicked "Clear all" button then there should be only one slot that is empty

if clicked "Delete" button and if current slot is not first, then should be destroyed

if clicked "Delete" burron on first slot, but there are other slots, then first slot is the second one, previous first slot is destroyed

if clicked "Delete" button on first slot and there aren't any other slots, then the first slot is not destroyed but is empty

Toggle->checkbox is not disabled

if Turbo->checkbox is not checked then turboSlider is disabled

if Turbo->checkbox is checked then turboSlider is enabled

if Turbo->delay label is equal to sliderValue / 100.0 + sec.

if Turbo->rate is equal to 100.0 / sliderValue + /sec.

if Set Selector->comboBox->count == 22

enum SlotTypeComboIndex {
        KBMouseSlot = 0, CycleSlot, DelaySlot, DistanceSlot, ExecuteSlot,
        HoldSlot, LoadSlot, MouseModSlot, PauseSlot, PressTimeSlot,
        ReleaseSlot, SetChangeSlot, TextEntry
    };

if SlotTypeComboIndex::KBMouseSlot == 0, SlotTypeComboIndex::CycleSlot == 1 itd

if last empty slot clicked and press some keyboard value it should hold new value from keyboard

if clicked "Clear all" button then there should be only one slot that is empty

if clicked "Delete" button and if current slot is not first, then should be destroyed

if clicked "Delete" burron on first slot, but there are other slots, then first slot is the second one, previous first slot is destroyed

if clicked "Delete" button on first slot and there aren't any other slots, then the first slot is not destroyed but is empty

Toggle->checkbox is not disabled

if Turbo->checkbox is not checked then turboSlider is disabled

if Turbo->checkbox is checked then turboSlider is enabled

if Turbo->delay label is equal to sliderValue / 100.0 + sec.

if Turbo->rate is equal to 100.0 / sliderValue + /sec.

if Set Selector->comboBox->count == 22

enum SlotTypeComboIndex {
        KBMouseSlot = 0, CycleSlot, DelaySlot, DistanceSlot, ExecuteSlot,
        HoldSlot, LoadSlot, MouseModSlot, PauseSlot, PressTimeSlot,
        ReleaseSlot, SetChangeSlot, TextEntry
    };

if SlotTypeComboIndex::KBMouseSlot == 0, SlotTypeComboIndex::CycleSlot == 1 itd

if slot->getSlotMode() == JoyButtonSlot::JoySetChange then ui->slotSetChangeComboBox->findData(QVariant(slot->getSlotCode())) >= 0

close/cancel button cannot be disabled

*/


class TestAdvanceButtonDialog: public QObject
{
    Q_OBJECT

public:
    TestAdvanceButtonDialog(QObject* parent = 0);

private slots:

private:
    AdvanceButtonDialog advanceButtonDialog;
};

TestAdvanceButtonDialog::TestAdvanceButtonDialog(QObject* parent) :
    QObject(parent),
    advanceButtonDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAdvanceButtonDialog)
#include "testadvancebuttondialog.moc"
