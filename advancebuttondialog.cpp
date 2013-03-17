#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollBar>
#include <QDebug>

#include "advancebuttondialog.h"
#include "ui_advancebuttondialog.h"
#include "event.h"

AdvanceButtonDialog::AdvanceButtonDialog(ButtonTempConfig *tempconfig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvanceButtonDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->tempconfig = tempconfig;
    oldRow = 0;

    if (this->tempconfig->toggle)
    {
        ui->toggleCheckbox->setChecked(true);
    }

    if (this->tempconfig->turbo)
    {
        ui->turboCheckbox->setChecked(true);
        ui->turboSlider->setEnabled(true);
    }

    int interval = (int)(this->tempconfig->turboInterval / 100);
    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot*> iter(*(tempconfig->assignments));
    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        if (buttonslot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            existingCode->setText(keycodeToKey(buttonslot->getSlotCode()).toUpper());
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyMouseButton)
        {
            existingCode->setText(QString("Mouse %1").arg(buttonslot->getSlotCode()));
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement)
        {
            existingCode->setText(buttonslot->movementString());
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyPause)
        {
            existingCode->setText(QString("Pause ").append(QString::number(buttonslot->getSlotCode() / 1000.0, 'g', 3)));
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyHold)
        {
            existingCode->setText(QString("Hold ").append(QString::number(buttonslot->getSlotCode() / 1000.0, 'g', 3)));
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            existingCode->setText("Cycle");
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
        {
            QString temp("Distance ");
            temp.append(QString::number(buttonslot->getSlotCode())).append("%");

            existingCode->setText(temp);
            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        }

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

    ui->horizSpinBox->setValue(tempconfig->mouseSpeedX);
    updateHorizSpeedLabel(tempconfig->mouseSpeedX);

    ui->vertSpinBox->setValue(tempconfig->mouseSpeedY);
    updateVertiSpeedLabel(tempconfig->mouseSpeedY);

    if (tempconfig->mouseSpeedX == tempconfig->mouseSpeedY)
    {
        ui->changeTogetherCheckBox->setChecked(true);
    }
    else
    {
        ui->changeTogetherCheckBox->setChecked(false);
    }

    if (tempconfig->setSelection > -1 && tempconfig->setSelectionCondition != JoyButton::SetChangeDisabled)
    {
        int offset = (int)tempconfig->setSelectionCondition;
        ui->setSelectionComboBox->setCurrentIndex((tempconfig->setSelection * 3) + offset);
    }

    if (tempconfig->originset == 0)
    {
        ui->setSelectionComboBox->model()->removeRows(1, 3);
    }
    else if (tempconfig->originset == 1)
    {
        ui->setSelectionComboBox->model()->removeRows(4, 3);
    }
    else if (tempconfig->originset == 2)
    {
        ui->setSelectionComboBox->model()->removeRows(7, 3);
    }
    else if (tempconfig->originset == 3)
    {
        ui->setSelectionComboBox->model()->removeRows(10, 3);
    }
    else if (tempconfig->originset == 4)
    {
        ui->setSelectionComboBox->model()->removeRows(13, 3);
    }
    else if (tempconfig->originset == 5)
    {
        ui->setSelectionComboBox->model()->removeRows(16, 3);
    }
    else if (tempconfig->originset == 6)
    {
        ui->setSelectionComboBox->model()->removeRows(19, 3);
    }
    else if (tempconfig->originset == 7)
    {
        ui->setSelectionComboBox->model()->removeRows(22, 3);
    }

    updateActionTimeLabel();
    changeTurboForSequences();

    connect(this, SIGNAL(accepted()), this, SLOT(updateTempConfig()));
    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), ui->turboSlider, SLOT(setEnabled(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(changeTurboText(int)));
    connect(ui->horizSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizSpeedLabel(int)));
    connect(ui->horizSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));

    connect(ui->vertSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVertiSpeedLabel(int)));
    connect(ui->vertSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));

    connect(ui->mouseUpPushButton, SIGNAL(clicked()), this, SLOT(addUpMovementSlot()));
    connect(ui->mouseDownPushButton, SIGNAL(clicked()), this, SLOT(addDownMovementSlot()));
    connect(ui->mouseLeftPushButton, SIGNAL(clicked()), this, SLOT(addLeftMovementSlot()));
    connect(ui->mouseRightPushButton, SIGNAL(clicked()), this, SLOT(addRightMovementSlot()));
    connect(ui->changeTogetherCheckBox, SIGNAL(clicked(bool)), this, SLOT(syncSpeedSpinBoxes()));

    connect(ui->insertSlotButton, SIGNAL(clicked()), this, SLOT(insertSlot()));
    connect(ui->deleteSlotButton, SIGNAL(clicked()), this, SLOT(deleteSlot()));
    connect(ui->clearAllPushButton, SIGNAL(clicked()), this, SLOT(clearAllSlots()));
    connect(ui->pausePushButton, SIGNAL(clicked()), this, SLOT(insertPauseSlot()));
    connect(ui->holdPushButton, SIGNAL(clicked()), this, SLOT(insertHoldSlot()));
    connect(ui->cyclePushButton, SIGNAL(clicked()), this, SLOT(insertCycleSlot()));
    connect(ui->distancePushButton, SIGNAL(clicked()), this, SLOT(insertDistanceSlot()));

    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMillisecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
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

void AdvanceButtonDialog::updateTempConfig()
{
    tempconfig->turbo = ui->turboCheckbox->isChecked();
    tempconfig->turboInterval = ui->turboSlider->value() * 100;
    tempconfig->toggle = ui->toggleCheckbox->isChecked();
    tempconfig->assignments->clear();

    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if (tempbuttonslot->getSlotCode() > 0)
        {
            JoyButtonSlot *buttonslot = new JoyButtonSlot(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
            tempconfig->assignments->append(buttonslot);
        }
    }

    tempconfig->mouseSpeedX = ui->horizSpinBox->value();
    tempconfig->mouseSpeedY = ui->vertSpinBox->value();

    if (ui->setSelectionComboBox->currentIndex() > 0)
    {
        int condition_choice = 0;
        int chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
        // Above removed rows
        if (tempconfig->originset > chosen_set)
        {
            tempconfig->setSelection = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
            condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;

        }
        // Below removed rows
        else
        {
            tempconfig->setSelection = (ui->setSelectionComboBox->currentIndex() + 2) / 3;
            condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;
        }

        //qDebug() << "CONDITION: " << QString::number(condition_choice) << endl;
        if (condition_choice == 0)
        {
            tempconfig->setSelectionCondition = JoyButton::SetChangeOneWay;
        }
        else if (condition_choice == 1)
        {
            tempconfig->setSelectionCondition = JoyButton::SetChangeTwoWay;
        }
        else if (condition_choice == 2)
        {
            tempconfig->setSelectionCondition = JoyButton::SetChangeWhileHeld;
        }
    }
    else
    {
        tempconfig->setSelection = -1;
        tempconfig->setSelectionCondition = JoyButton::SetChangeDisabled;
    }

    //qDebug() << "SET SELECTION: " << QString::number(tempconfig->setSelection) << endl;
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

    changeTurboForSequences();
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

void AdvanceButtonDialog::updateHorizSpeedLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(20.0 * value)).append(" pps");
    ui->horizSpeedLabel->setText(label);
}

void AdvanceButtonDialog::updateVertiSpeedLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(20.0 * value)).append(" pps");
    ui->vertiSpeedLabel->setText(label);
}

void AdvanceButtonDialog::addUpMovementSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement);
    updateSlotsScrollArea(JoyButtonSlot::MouseUp);
}

void AdvanceButtonDialog::addDownMovementSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement);
    updateSlotsScrollArea(JoyButtonSlot::MouseDown);
}

void AdvanceButtonDialog::addLeftMovementSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement);
    updateSlotsScrollArea(JoyButtonSlot::MouseLeft);
}

void AdvanceButtonDialog::addRightMovementSlot()
{
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement);
    updateSlotsScrollArea(JoyButtonSlot::MouseRight);
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

void AdvanceButtonDialog::moveSpeedsTogether(int value)
{
    if (ui->changeTogetherCheckBox->isChecked())
    {
        ui->horizSpinBox->setValue(value);
        ui->vertSpinBox->setValue(value);
    }
}

void AdvanceButtonDialog::syncSpeedSpinBoxes()
{
    int temp = ui->horizSpinBox->value();
    if (temp > ui->vertSpinBox->value())
    {
        ui->vertSpinBox->setValue(temp);
    }
    else
    {
        temp = ui->vertSpinBox->value();
        ui->horizSpinBox->setValue(temp);
    }
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
        ui->turboCheckbox->setChecked(false);
        ui->turboCheckbox->setEnabled(false);
    }
    else
    {
        ui->turboCheckbox->setEnabled(true);
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
