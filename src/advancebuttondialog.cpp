#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>
#include <cmath>

#include "advancebuttondialog.h"
#include "ui_advancebuttondialog.h"
#include "event.h"
#include "setjoystick.h"

const int AdvanceButtonDialog::MINIMUMTURBO = 2;

AdvanceButtonDialog::AdvanceButtonDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::AdvanceButtonDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->button = button;
    oldRow = 0;

    if (this->button->getToggleState())
    {
        ui->toggleCheckbox->setChecked(true);
    }

    if (this->button->isUsingTurbo())
    {
        ui->turboCheckbox->setChecked(true);
        ui->turboSlider->setEnabled(true);
    }

    int interval = this->button->getTurboInterval() / 10;
    if (interval < MINIMUMTURBO)
    {
        interval = JoyButton::ENABLEDTURBODEFAULT / 10;
    }
    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot*> iter(*(this->button->getAssignedSlots()));
    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        existingCode->setText(buttonslot->getSlotString());
        existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotCodeAlias(), buttonslot->getSlotMode());

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(existingCode));
        QHBoxLayout *layout= new QHBoxLayout();
        layout->setContentsMargins(10, 0, 10, 0);
        layout->addWidget(existingCode);
        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        item->setSizeHint(widget->sizeHint());

        ui->slotListWidget->addItem(item);
        ui->slotListWidget->setItemWidget(item, widget);
        connectButtonEvents(existingCode);
    }

    appendBlankKeyGrabber();

    if (this->button->getSetSelection() > -1 && this->button->getChangeSetCondition() != JoyButton::SetChangeDisabled)
    {
        int offset = (int)this->button->getChangeSetCondition();
        ui->setSelectionComboBox->setCurrentIndex((this->button->getSetSelection() * 3) + offset);
    }

    if (this->button->getOriginSet() == 0)
    {
        ui->setSelectionComboBox->model()->removeRows(1, 3);
    }
    else if (this->button->getOriginSet() == 1)
    {
        ui->setSelectionComboBox->model()->removeRows(4, 3);
    }
    else if (this->button->getOriginSet() == 2)
    {
        ui->setSelectionComboBox->model()->removeRows(7, 3);
    }
    else if (this->button->getOriginSet() == 3)
    {
        ui->setSelectionComboBox->model()->removeRows(10, 3);
    }
    else if (this->button->getOriginSet() == 4)
    {
        ui->setSelectionComboBox->model()->removeRows(13, 3);
    }
    else if (this->button->getOriginSet() == 5)
    {
        ui->setSelectionComboBox->model()->removeRows(16, 3);
    }
    else if (this->button->getOriginSet() == 6)
    {
        ui->setSelectionComboBox->model()->removeRows(19, 3);
    }
    else if (this->button->getOriginSet() == 7)
    {
        ui->setSelectionComboBox->model()->removeRows(22, 3);
    }

    fillTimeComboBoxes();
    ui->actionTenthsComboBox->setCurrentIndex(1);

    updateActionTimeLabel();
    changeTurboForSequences();

    if (button->isCycleResetActive())
    {
        ui->autoResetCycleCheckBox->setEnabled(true);
        ui->autoResetCycleCheckBox->setChecked(true);
        checkCycleResetWidgetStatus(true);
    }

    if (button->getCycleResetTime() != 0)
    {
        populateAutoResetInterval();
    }

    updateWindowTitleButtonName();

    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), ui->turboSlider, SLOT(setEnabled(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(checkTurboIntervalValue(int)));

    connect(ui->insertSlotButton, SIGNAL(clicked()), this, SLOT(insertSlot()));
    connect(ui->deleteSlotButton, SIGNAL(clicked()), this, SLOT(deleteSlot()));
    connect(ui->clearAllPushButton, SIGNAL(clicked()), this, SLOT(clearAllSlots()));
    connect(ui->pausePushButton, SIGNAL(clicked()), this, SLOT(insertPauseSlot()));
    connect(ui->holdPushButton, SIGNAL(clicked()), this, SLOT(insertHoldSlot()));
    connect(ui->cyclePushButton, SIGNAL(clicked()), this, SLOT(insertCycleSlot()));
    connect(ui->distancePushButton, SIGNAL(clicked()), this, SLOT(insertDistanceSlot()));
    connect(ui->releasePushButton, SIGNAL(clicked()), this, SLOT(insertReleaseSlot()));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));

    connect(ui->toggleCheckbox, SIGNAL(clicked(bool)), button, SLOT(setToggle(bool)));
    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), this, SLOT(checkTurboSetting(bool)));

    connect(ui->setSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSetSelection()));
    connect(ui->mouseModPushButton, SIGNAL(clicked()), this, SLOT(insertMouseSpeedModSlot()));

    connect(ui->slotListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(performStatsWidgetRefresh(QListWidgetItem*)));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));

    connect(ui->distanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotDistanceUpdate()));
    connect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotMouseModUpdate()));
    connect(ui->pressTimePushButton, SIGNAL(clicked()), this, SLOT(insertKeyPressSlot()));
    connect(ui->delayPushButton, SIGNAL(clicked()), this, SLOT(insertDelaySlot()));

    connect(ui->autoResetCycleCheckBox, SIGNAL(clicked(bool)), this, SLOT(checkCycleResetWidgetStatus(bool)));
    connect(ui->autoResetCycleCheckBox, SIGNAL(clicked(bool)), this, SLOT(setButtonCycleReset(bool)));
    connect(ui->resetCycleDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setButtonCycleResetInterval(double)));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckbox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
}

AdvanceButtonDialog::~AdvanceButtonDialog()
{
    delete ui;
}

void AdvanceButtonDialog::changeTurboText(int value)
{
    if (value >= MINIMUMTURBO)
    {
        double delay = value / 100.0;
        double clicks = 100.0 / (double)value;
        QString delaytext = QString::number(delay, 'g', 3).append(" ").append(tr("sec."));
        QString labeltext = QString::number(clicks, 'g', 2).append(" ").append(tr("/sec."));

        ui->delayValueLabel->setText(delaytext);
        ui->rateValueLabel->setText(labeltext);
    }
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    if (index == (itemcount - 1) && value >= 0)
    {
        // New slot added on the old blank button. Append
        // new blank button to the end of the list.
        appendBlankKeyGrabber();
    }

    // Go through all grabber buttons in list and possibly resize widgets.
    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->slotListWidget->item(i);
        QWidget *widget = ui->slotListWidget->itemWidget(item);
        item->setSizeHint(widget->sizeHint());
    }

    // Alter interface if turbo cannot be used.
    changeTurboForSequences();

    emit slotsChanged();
}

void AdvanceButtonDialog::connectButtonEvents(SimpleKeyGrabberButton *button)
{
    connect(button, SIGNAL(clicked()), this, SLOT(changeSelectedSlot()));
    connect(button, SIGNAL(buttonCodeChanged(int)), this, SLOT(updateSelectedSlot(int)));
    //connect(button, SIGNAL(buttonCodeChanged(int)), this, SLOT(updateSlotsScrollArea(int)));
}

void AdvanceButtonDialog::updateSelectedSlot(int value)
{
    SimpleKeyGrabberButton *grabbutton = static_cast<SimpleKeyGrabberButton*>(sender());
    JoyButtonSlot *tempbuttonslot = grabbutton->getValue();
    int index = ui->slotListWidget->currentRow();

    // Stop all events on JoyButton
    this->button->eventReset();

    this->button->setAssignedSlot(tempbuttonslot->getSlotCode(),
                                  tempbuttonslot->getSlotCodeAlias(),
                                  index,
                                  tempbuttonslot->getSlotMode());
    updateSlotsScrollArea(value);
}

void AdvanceButtonDialog::deleteSlot()
{
    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    QListWidgetItem *item = ui->slotListWidget->takeItem(index);
    delete item;
    item = 0;

    // Deleted last button. Replace with new blank button
    if (index == itemcount - 1)
    {
        appendBlankKeyGrabber();
    }

    changeTurboForSequences();

    // Stop all events on JoyButton
    button->eventReset();

    button->removeAssignedSlot(index);
    emit slotsChanged();
}

void AdvanceButtonDialog::changeSelectedSlot()
{
    SimpleKeyGrabberButton *button = static_cast<SimpleKeyGrabberButton*>(sender());

    bool leave = false;
    for (int i = 0; i < ui->slotListWidget->count() && !leave; i++)
    {
        QListWidgetItem *item = ui->slotListWidget->item(i);
        SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        if (button == tempbutton)
        {
            ui->slotListWidget->setCurrentRow(i);
            leave = true;
            oldRow = i;
        }
    }
}

void AdvanceButtonDialog::appendBlankKeyGrabber()
{
    SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(blankButton));

    QHBoxLayout *layout= new QHBoxLayout();
    layout->setContentsMargins(10, 0, 10, 0);
    layout->addWidget(blankButton);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    item->setSizeHint(widget->sizeHint());

    ui->slotListWidget->addItem(item);
    ui->slotListWidget->setItemWidget(item, widget);
    ui->slotListWidget->setCurrentItem(item);
    connectButtonEvents(blankButton);
}

void AdvanceButtonDialog::insertSlot()
{
    int current = ui->slotListWidget->currentRow();
    int count = ui->slotListWidget->count();

    if (current != (count - 1))
    {
        SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
        QListWidgetItem *item = new QListWidgetItem();
        ui->slotListWidget->insertItem(current, item);
        item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(blankButton));
        QHBoxLayout *layout= new QHBoxLayout();
        layout->addWidget(blankButton);
        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        item->setSizeHint(widget->sizeHint());
        ui->slotListWidget->setItemWidget(item, widget);
        ui->slotListWidget->setCurrentItem(item);
        connectButtonEvents(blankButton);
        blankButton->refreshButtonLabel();

        this->button->insertAssignedSlot(0, 0, current);
    }
}

void AdvanceButtonDialog::insertPauseSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime >= 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyPause);
        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(actionTime, 0, index, JoyButtonSlot::JoyPause);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertReleaseSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyRelease);
        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(actionTime, 0, index, JoyButtonSlot::JoyRelease);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertHoldSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyHold);
        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(actionTime, 0, index, JoyButtonSlot::JoyHold);
        updateSlotsScrollArea(actionTime);
    }
}

int AdvanceButtonDialog::actionTimeConvert()
{
    int minutesIndex = ui->actionMinutesComboBox->currentIndex();
    int secondsIndex = ui->actionSecondsComboBox->currentIndex();
    int hundredthsIndex = ui->actionHundredthsComboBox->currentIndex();
    int tenthsIndex = ui->actionTenthsComboBox->currentIndex();

    int tempMilliSeconds = minutesIndex * 1000 * 60;
    tempMilliSeconds += secondsIndex * 1000;
    tempMilliSeconds += tenthsIndex * 100;
    tempMilliSeconds += hundredthsIndex * 10;
    return tempMilliSeconds;
}

void AdvanceButtonDialog::refreshTimeComboBoxes(JoyButtonSlot *slot)
{
    disconnectTimeBoxesEvents();

    int slottime = slot->getSlotCode();
    int tempMinutes = slottime / 1000 / 60;
    int tempSeconds = slottime / 1000 % 60;
    int tempTenthsSeconds = (slottime % 1000) / 100;
    int tempHundredthsSeconds = (slottime % 1000 % 100) / 10;

    ui->actionMinutesComboBox->setCurrentIndex(tempMinutes);
    ui->actionSecondsComboBox->setCurrentIndex(tempSeconds);
    ui->actionTenthsComboBox->setCurrentIndex(tempTenthsSeconds);
    ui->actionHundredthsComboBox->setCurrentIndex(tempHundredthsSeconds);
    updateActionTimeLabel();

    connectTimeBoxesEvents();
}

void AdvanceButtonDialog::updateActionTimeLabel()
{
    int actionTime = actionTimeConvert();
    int minutes = actionTime / 1000 / 60;
    double hundredths = actionTime % 1000 / 1000.0;
    double seconds = (actionTime / 1000 % 60) + hundredths;
    QString temp;
    temp.append(QString::number(minutes)).append("m ");
    temp.append(QString::number(seconds, 'f', 2)).append("s");
    ui->actionTimeLabel->setText(temp);
}

void AdvanceButtonDialog::clearAllSlots()
{
    ui->slotListWidget->clear();
    appendBlankKeyGrabber();
    changeTurboForSequences();

    button->clearSlotsEventReset();
    emit slotsChanged();
}

void AdvanceButtonDialog::changeTurboForSequences()
{
    bool containsSequences = false;
    for (int i = 0; i < ui->slotListWidget->count() && !containsSequences; i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if (tempbuttonslot->getSlotCode() > 0 &&
            (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyPause ||
             tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyHold ||
             tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance
            )
           )
        {
            containsSequences = true;
        }
    }

    if (containsSequences)
    {
        if (ui->turboCheckbox->isChecked())
        {
            ui->turboCheckbox->setChecked(false);
            this->button->setUseTurbo(false);
            emit turboChanged(false);
        }

        if (ui->turboCheckbox->isEnabled())
        {
            ui->turboCheckbox->setEnabled(false);
            emit turboButtonEnabledChange(false);
        }
    }
    else
    {
        if (!ui->turboCheckbox->isEnabled())
        {
            ui->turboCheckbox->setEnabled(true);
            emit turboButtonEnabledChange(true);
        }
    }
}

void AdvanceButtonDialog::insertCycleSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(1, JoyButtonSlot::JoyCycle);

    // Stop all events on JoyButton
    this->button->eventReset();

    this->button->setAssignedSlot(1, 0, index, JoyButtonSlot::JoyCycle);
    updateSlotsScrollArea(1);
}

void AdvanceButtonDialog::insertDistanceSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();

    int tempDistance = 0;
    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
        {
            tempDistance += tempbuttonslot->getSlotCode();
        }
        else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            tempDistance = 0;
        }
    }

    int testDistance = ui->distanceSpinBox->value();
    if (testDistance + tempDistance <= 100)
    {
        tempbutton->setValue(testDistance, JoyButtonSlot::JoyDistance);

        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(testDistance, 0, index, JoyButtonSlot::JoyDistance);
        updateSlotsScrollArea(testDistance);
    }
}

void AdvanceButtonDialog::placeNewSlot(JoyButtonSlot *slot)
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(slot->getSlotCode(), slot->getSlotCodeAlias(), slot->getSlotMode());

    // Stop all events on JoyButton
    this->button->eventReset();

    this->button->setAssignedSlot(slot->getSlotCode(), slot->getSlotCodeAlias(), index, slot->getSlotMode());
    updateSlotsScrollArea(slot->getSlotCode());
    slot->deleteLater();
}

void AdvanceButtonDialog::updateTurboIntervalValue(int value)
{
    if (value >= MINIMUMTURBO)
    {
        button->setTurboInterval(value * 10);
    }
}

void AdvanceButtonDialog::checkTurboSetting(bool state)
{
    ui->turboCheckbox->setChecked(state);
    ui->turboSlider->setEnabled(state);
    changeTurboForSequences();
    button->setUseTurbo(state);
    if (button->getTurboInterval() / 10 >= MINIMUMTURBO)
    {
        ui->turboSlider->setValue(button->getTurboInterval() / 10);
    }
}

void AdvanceButtonDialog::updateSetSelection()
{
    JoyButton::SetChangeCondition oldCondition = button->getChangeSetCondition();
    int condition_choice = 0;
    JoyButton::SetChangeCondition set_selection_condition = JoyButton::SetChangeDisabled;
    int chosen_set = -1;


    if (ui->setSelectionComboBox->currentIndex() > 0)
    {
        condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;
        chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;

        // Above removed rows
        if (button->getOriginSet() > chosen_set)
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
        }
        // Below removed rows
        else
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() + 2) / 3;
        }

        //qDebug() << "CONDITION: " << QString::number(condition_choice) << endl;
        if (condition_choice == 0)
        {
            set_selection_condition = JoyButton::SetChangeOneWay;
        }
        else if (condition_choice == 1)
        {
            set_selection_condition = JoyButton::SetChangeTwoWay;
        }
        else if (condition_choice == 2)
        {
            set_selection_condition = JoyButton::SetChangeWhileHeld;
        }
        //qDebug() << "CHOSEN SET: " << chosen_set << endl;
    }
    else
    {
        chosen_set = -1;
        set_selection_condition = JoyButton::SetChangeDisabled;
    }

    if (chosen_set > -1 && set_selection_condition != JoyButton::SetChangeDisabled)
    {
        // Revert old set condition before entering new set condition.
        // Also, do not emit signals on first change
        button->setChangeSetCondition(oldCondition, true);

        button->setChangeSetSelection(chosen_set);
        button->setChangeSetCondition(set_selection_condition);
    }
    else
    {
        button->setChangeSetSelection(-1);
        button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    }
}

void AdvanceButtonDialog::checkTurboIntervalValue(int value)
{
    if (value >= MINIMUMTURBO)
    {
        changeTurboText(value);
        updateTurboIntervalValue(value);
    }
    else
    {
        ui->turboSlider->setValue(MINIMUMTURBO);
    }
}

void AdvanceButtonDialog::fillTimeComboBoxes()
{
    ui->actionMinutesComboBox->clear();
    ui->actionSecondsComboBox->clear();
    ui->actionHundredthsComboBox->clear();
    ui->actionTenthsComboBox->clear();

    for (double i=0; i <= 10; i++)
    {
        QString temp = QString::number(i, 'g', 2).append("m");
        ui->actionMinutesComboBox->addItem(temp);
    }

    for (double i=0; i <= 59; i++)
    {
        QString temp = QString::number(i, 'g', 2);
        ui->actionSecondsComboBox->addItem(temp);
    }

    for (int i=0; i < 10; i++)
    {
        QString temp = QString(".%1").arg(i, 1, 10, QChar('0'));
        ui->actionTenthsComboBox->addItem(temp);
    }

    for (int i=0; i < 10; i++)
    {
        QString temp = QString("%1s").arg(i, 1, 10, QChar('0'));
        ui->actionHundredthsComboBox->addItem(temp);
    }
}

void AdvanceButtonDialog::insertMouseSpeedModSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int tempMouseMod = ui->mouseSpeedModSpinBox->value();
    if (tempMouseMod > 0)
    {
        tempbutton->setValue(tempMouseMod, JoyButtonSlot::JoyMouseSpeedMod);

        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(tempMouseMod, 0, index, JoyButtonSlot::JoyMouseSpeedMod);
        updateSlotsScrollArea(tempMouseMod);
    }
}

void AdvanceButtonDialog::insertKeyPressSlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyKeyPress);

        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(actionTime, 0, index, JoyButtonSlot::JoyKeyPress);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertDelaySlot()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyDelay);
        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(actionTime, 0, index, JoyButtonSlot::JoyDelay);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::performStatsWidgetRefresh(QListWidgetItem *item)
{
    SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *slot = tempbutton->getValue();

    if (slot->getSlotMode() == JoyButtonSlot::JoyPause)
    {
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyRelease)
    {
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyKeyPress)
    {
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDelay)
    {
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        disconnect(ui->distanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotDistanceUpdate()));
        ui->distanceSpinBox->setValue(slot->getSlotCode());
        connect(ui->distanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotDistanceUpdate()));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod)
    {
        disconnect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotMouseModUpdate()));
        ui->mouseSpeedModSpinBox->setValue(slot->getSlotCode());
        connect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotMouseModUpdate()));
    }
}

void AdvanceButtonDialog::checkSlotTimeUpdate()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *tempbuttonslot = tempbutton->getValue();
    if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyPause ||
        tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyHold ||
        tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyRelease ||
        tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyKeyPress ||
        tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDelay)
    {
        int actionTime = actionTimeConvert();
        if (actionTime > 0)
        {
            tempbutton->setValue(actionTime, tempbuttonslot->getSlotMode());

            // Stop all events on JoyButton
            this->button->eventReset();

            this->button->setAssignedSlot(actionTime, 0, index, tempbuttonslot->getSlotMode());
            updateSlotsScrollArea(actionTime);
        }
    }
}

void AdvanceButtonDialog::checkSlotMouseModUpdate()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *tempbuttonslot = tempbutton->getValue();

    int tempMouseMod = ui->mouseSpeedModSpinBox->value();
    if (tempMouseMod > 0)
    {
        //tempbuttonslot->setSlotCode(tempMouseMod);
        tempbutton->setValue(tempMouseMod, tempbuttonslot->getSlotMode());

        // Stop all events on JoyButton
        this->button->eventReset();

        this->button->setAssignedSlot(tempMouseMod, 0, index, tempbuttonslot->getSlotMode());
        updateSlotsScrollArea(tempMouseMod);
    }
}

void AdvanceButtonDialog::checkSlotDistanceUpdate()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();
    int tempDistance = 0;

    if (buttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        for (int i = 0; i < ui->slotListWidget->count(); i++)
        {
            SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
            JoyButtonSlot *tempbuttonslot = button->getValue();
            if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
            {
                tempDistance += tempbuttonslot->getSlotCode();
            }
            else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
            {
                tempDistance = 0;
            }
        }

        int testDistance = ui->distanceSpinBox->value();
        tempDistance += testDistance - buttonslot->getSlotCode();
        if (tempDistance <= 100)
        {
            //buttonslot->setSlotCode(testDistance);
            tempbutton->setValue(testDistance, buttonslot->getSlotMode());

            // Stop all events on JoyButton
            this->button->eventReset();

            this->button->setAssignedSlot(testDistance, 0, index, buttonslot->getSlotMode());
            updateSlotsScrollArea(testDistance);
        }
    }
}

void AdvanceButtonDialog::updateWindowTitleButtonName()
{
    QString temp;
    temp.append(tr("Advanced").append(": ")).append(button->getPartialName(false, true));

    if (button->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = button->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = button->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void AdvanceButtonDialog::checkCycleResetWidgetStatus(bool enabled)
{
    if (enabled)
    {
        ui->resetCycleDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->resetCycleDoubleSpinBox->setEnabled(false);
    }
}

void AdvanceButtonDialog::setButtonCycleResetInterval(double value)
{
    unsigned int milliseconds = ((int)value * 1000) + (fmod(value, 1.0) * 1000);
    button->setCycleResetTime(milliseconds);
}

void AdvanceButtonDialog::populateAutoResetInterval()
{
    double seconds = button->getCycleResetTime() / 1000.0;
    ui->resetCycleDoubleSpinBox->setValue(seconds);
}

void AdvanceButtonDialog::setButtonCycleReset(bool enabled)
{
    if (enabled)
    {
        button->setCycleResetStatus(true);
        if (button->getCycleResetTime() == 0 && ui->resetCycleDoubleSpinBox->value() > 0.0)
        {
            double current = ui->resetCycleDoubleSpinBox->value();
            setButtonCycleResetInterval(current);
        }
    }
    else
    {
        button->setCycleResetStatus(false);
    }
}

void AdvanceButtonDialog::resetTimeBoxes()
{
    disconnectTimeBoxesEvents();

    ui->actionMinutesComboBox->setCurrentIndex(0);
    ui->actionSecondsComboBox->setCurrentIndex(0);
    ui->actionTenthsComboBox->setCurrentIndex(1);
    ui->actionHundredthsComboBox->setCurrentIndex(0);

    updateActionTimeLabel();
    connectTimeBoxesEvents();
}

void AdvanceButtonDialog::disconnectTimeBoxesEvents()
{
    disconnect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));

    disconnect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
}

void AdvanceButtonDialog::connectTimeBoxesEvents()
{
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
}
