#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QDebug>

#include "advancebuttondialog.h"
#include "ui_advancebuttondialog.h"
#include "event.h"

AdvanceButtonDialog::AdvanceButtonDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Window),
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

    int interval = (int)(this->button->getTurboInterval() / 100);
    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot*> iter(*(this->button->getAssignedSlots()));
    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        existingCode->setText(buttonslot->getSlotString());
        existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());

        //existingCode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        connectButtonEvents(existingCode);
        QListWidgetItem *item = new QListWidgetItem();
        ui->slotListWidget->addItem(item);

        item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(existingCode));
        QHBoxLayout *layout= new QHBoxLayout();
        layout->addWidget(existingCode);
        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        item->setSizeHint(widget->sizeHint());
        ui->slotListWidget->setItemWidget(item, widget);
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

    updateActionTimeLabel();
    changeTurboForSequences();

    setWindowTitle(tr("Advanced").append(": ").append(button->getPartialName()));

    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), ui->turboSlider, SLOT(setEnabled(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(changeTurboText(int)));

    connect(ui->insertSlotButton, SIGNAL(clicked()), this, SLOT(insertSlot()));
    connect(ui->deleteSlotButton, SIGNAL(clicked()), this, SLOT(deleteSlot()));
    connect(ui->clearAllPushButton, SIGNAL(clicked()), this, SLOT(clearAllSlots()));
    connect(ui->pausePushButton, SIGNAL(clicked()), this, SLOT(insertPauseSlot()));
    connect(ui->holdPushButton, SIGNAL(clicked()), this, SLOT(insertHoldSlot()));
    connect(ui->cyclePushButton, SIGNAL(clicked()), this, SLOT(insertCycleSlot()));
    connect(ui->distancePushButton, SIGNAL(clicked()), this, SLOT(insertDistanceSlot()));
    connect(ui->releasePushButton, SIGNAL(clicked()), this, SLOT(insertReleaseSlot()));

    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMillisecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));

    connect(ui->toggleCheckbox, SIGNAL(clicked(bool)), button, SLOT(setToggle(bool)));
    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), this, SLOT(checkTurboSetting(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(updateTurboIntervalValue(int)));

    connect(ui->setSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSetSelection()));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckbox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
}

AdvanceButtonDialog::~AdvanceButtonDialog()
{
    delete ui;
}

void AdvanceButtonDialog::changeTurboText(int value)
{
    if (value == 0)
    {
        value = 1;
    }

    double delay = value / 10.0;
    double clicks = 100 / (value * 10.0);
    QString delaytext = QString(QString::number(delay, 'g', 3)).append(" sec.");
    QString labeltext = QString(QString::number(clicks, 'g', 2)).append("/sec.");

    ui->delayValueLabel->setText(delaytext);
    ui->rateValueLabel->setText(labeltext);
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
    //ui->slotListWidget->setCurrentRow(oldRow);
    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    if (index == (itemcount - 1) && value > 0)
    {
        appendBlankKeyGrabber();
    }
    else if (index < (itemcount - 1) && value == 0)
    {
        QListWidgetItem *item = ui->slotListWidget->takeItem(index);
        delete item;
        item = 0;
    }

    this->button->clearSlotsEventReset();
    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if (tempbuttonslot->getSlotCode() > 0)
        {
            JoyButtonSlot *buttonslot = new JoyButtonSlot(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
            this->button->setAssignedSlot(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
    }

    changeTurboForSequences();

    emit slotsChanged();
}

void AdvanceButtonDialog::connectButtonEvents(SimpleKeyGrabberButton *button)
{
    connect(button, SIGNAL(clicked()), this, SLOT(changeSelectedSlot()));
    connect(button, SIGNAL(buttonCodeChanged(int)), this, SLOT(updateSlotsScrollArea(int)));
}

void AdvanceButtonDialog::deleteSlot()
{
    int index = ui->slotListWidget->currentIndex().row();
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
    ui->slotListWidget->addItem(item);
    item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(blankButton));
    QHBoxLayout *layout= new QHBoxLayout();
    layout->addWidget(blankButton);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    item->setSizeHint(widget->sizeHint());
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
    }
}

void AdvanceButtonDialog::insertPauseSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyPause);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertReleaseSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyRelease);
        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertHoldSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyHold);
        updateSlotsScrollArea(actionTime);
    }
}

int AdvanceButtonDialog::actionTimeConvert()
{
    int tempSeconds = 0;
    int secondsIndex = ui->actionSecondsComboBox->currentIndex();
    int millisecondsIndex = ui->actionMillisecondsComboBox->currentIndex();
    switch (secondsIndex)
    {
        case 0: tempSeconds += 0; break;
        case 1: tempSeconds += 1000; break;
        case 2: tempSeconds += 2000; break;
        case 3: tempSeconds += 3000; break;
        case 4: tempSeconds += 4000; break;
        case 5: tempSeconds += 5000; break;
        case 6: tempSeconds += 6000; break;
        case 7: tempSeconds += 7000; break;
        case 8: tempSeconds += 8000; break;
        case 9: tempSeconds += 9000; break;
        case 10: tempSeconds += 10000; break;

        default: break;
    }

    switch (millisecondsIndex)
    {
        case 0: tempSeconds += 0; break;
        case 1: tempSeconds += 100; break;
        case 2: tempSeconds += 200; break;
        case 3: tempSeconds += 300; break;
        case 4: tempSeconds += 400; break;
        case 5: tempSeconds += 500; break;
        case 6: tempSeconds += 600; break;
        case 7: tempSeconds += 700; break;
        case 8: tempSeconds += 800; break;
        case 9: tempSeconds += 900; break;

        default: break;
    }

    return tempSeconds;
}

void AdvanceButtonDialog::updateActionTimeLabel()
{
    int actionTime = actionTimeConvert();
    QString temp("");
    temp.append(QString::number(actionTime / 1000.0, 'g', 5)).append("s");
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
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(1, JoyButtonSlot::JoyCycle);
    updateSlotsScrollArea(1);
}

void AdvanceButtonDialog::insertDistanceSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    /*int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyPause);
        updateSlotsScrollArea(actionTime);
    }*/

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

    /*QListIterator<JoyButtonSlot*> iter(tempconfig->assignments);
    int tempDistance = 0;
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
        {
            tempDistance += slot->getSlotCode();
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            tempDistance = 0;
        }
    }*/

    int testDistance = ui->distanceSpinBox->value();
    if (testDistance + tempDistance <= 100)
    {
        tempbutton->setValue(testDistance, JoyButtonSlot::JoyDistance);
        updateSlotsScrollArea(testDistance);
    }
}

void AdvanceButtonDialog::placeNewSlot(JoyButtonSlot *slot)
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(slot->getSlotCode(), slot->getSlotMode());
    updateSlotsScrollArea(slot->getSlotCode());
}

void AdvanceButtonDialog::updateTurboIntervalValue(int value)
{
    button->setTurboInterval(value * 100);
}

void AdvanceButtonDialog::checkTurboSetting(bool state)
{
    ui->turboCheckbox->setChecked(state);
    changeTurboForSequences();
    button->setUseTurbo(state);
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
