#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollBar>

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

    connect(this, SIGNAL(accepted()), this, SLOT(updateTempConfig()));
    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), ui->turboSlider, SLOT(setEnabled(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(changeTurboText(int)));
    connect(ui->deleteSlotButton, SIGNAL(clicked()), this, SLOT(deleteSlot()));
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
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
    ui->slotListWidget->setCurrentRow(oldRow);
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
