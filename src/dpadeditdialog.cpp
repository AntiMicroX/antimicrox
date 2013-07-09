#include <QHashIterator>

#include "dpadeditdialog.h"
#include "ui_dpadeditdialog.h"
#include "event.h"

DPadEditDialog::DPadEditDialog(JoyDPad *dpad, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::DPadEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->dpad = dpad;

    QString tempname;
    tempname.append(tr("Set")).append(" ");
    tempname.append(dpad->getName());
    this->setWindowTitle(tempname);

    if (dpad->getJoyMode() == JoyDPad::StandardMode)
    {
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (dpad->getJoyMode() == JoyDPad::EightWayMode)
    {
        ui->joyModeComboBox->setCurrentIndex(1);
    }

    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    int tempMouseSpeedX = 0;
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        tempMouseSpeedX = qMax(tempMouseSpeedX, button->getMouseSpeedX());
    }

    iter.toFront();
    int tempMouseSpeedY = 0;
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        tempMouseSpeedY = qMax(tempMouseSpeedY, button->getMouseSpeedY());
    }

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    updateHorizontalSpeedConvertLabel(tempMouseSpeedX);

    ui->verticalSpinBox->setValue(tempMouseSpeedY);
    updateVerticalSpeedConvertLabel(tempMouseSpeedY);

    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
    connect(ui->joyModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementModes(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));

    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->changeTogetherCheckBox, SIGNAL(clicked(bool)), this, SLOT(syncSpeedSpinBoxes()));
    connect(ui->changeMouseSpeedsCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeMouseSpeedsInterface(bool)));
}

DPadEditDialog::~DPadEditDialog()
{
    delete ui;
}

void DPadEditDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyButtonSlot::JOYSPEED * value)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void DPadEditDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyButtonSlot::JOYSPEED * value)).append(" pps");
    ui->verticalSpeedLabel->setText(label);
}

void DPadEditDialog::moveSpeedsTogether(int value)
{
    if (ui->changeTogetherCheckBox->isChecked())
    {
        ui->horizontalSpinBox->setValue(value);
        ui->verticalSpinBox->setValue(value);
    }
}

void DPadEditDialog::syncSpeedSpinBoxes()
{
    int temp = ui->horizontalSpinBox->value();
    if (temp > ui->verticalSpinBox->value())
    {
        ui->verticalSpinBox->setValue(temp);
    }
    else
    {
        temp = ui->verticalSpinBox->value();
        ui->horizontalSpinBox->setValue(temp);
    }
}

void DPadEditDialog::changeMouseSpeedsInterface(bool value)
{
    ui->horizontalSpinBox->setEnabled(value);
    ui->verticalSpinBox->setEnabled(value);
    ui->changeTogetherCheckBox->setEnabled(value);
}

void DPadEditDialog::updateConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            button->setMouseSpeedX(value);
        }
    }
}

void DPadEditDialog::updateConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            button->setMouseSpeedY(value);
        }
    }
}

void DPadEditDialog::implementPresets(int index)
{
    JoyButtonSlot *upButtonSlot = 0;
    JoyButtonSlot *downButtonSlot = 0;
    JoyButtonSlot *leftButtonSlot = 0;
    JoyButtonSlot *rightButtonSlot = 0;
    JoyButtonSlot *upLeftButtonSlot = 0;
    JoyButtonSlot *upRightButtonSlot = 0;
    JoyButtonSlot *downLeftButtonSlot = 0;
    JoyButtonSlot *downRightButtonSlot = 0;

    if (index == 1)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 2)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 3)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 4)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 5)
    {
        upButtonSlot = new JoyButtonSlot(keyToKeycode("Up"), JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(keyToKeycode("Down"), JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(keyToKeycode("Left"), JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(keyToKeycode("Right"), JoyButtonSlot::JoyKeyboard, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 6)
    {
        upButtonSlot = new JoyButtonSlot(keyToKeycode("w"), JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(keyToKeycode("s"), JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(keyToKeycode("a"), JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(keyToKeycode("d"), JoyButtonSlot::JoyKeyboard, this);
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 7)
    {
        if (ui->joyModeComboBox->currentIndex() == 0)
        {
            upButtonSlot = new JoyButtonSlot(keyToKeycode("KP_8"), JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(keyToKeycode("KP_2"), JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(keyToKeycode("KP_4"), JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(keyToKeycode("KP_6"), JoyButtonSlot::JoyKeyboard, this);
        }
        else if (ui->joyModeComboBox->currentIndex() == 1)
        {
            upButtonSlot = new JoyButtonSlot(keyToKeycode("KP_8"), JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(keyToKeycode("KP_2"), JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(keyToKeycode("KP_4"), JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(keyToKeycode("KP_6"), JoyButtonSlot::JoyKeyboard, this);

            upLeftButtonSlot = new JoyButtonSlot(keyToKeycode("KP_7"), JoyButtonSlot::JoyKeyboard, this);
            upRightButtonSlot = new JoyButtonSlot(keyToKeycode("KP_9"), JoyButtonSlot::JoyKeyboard, this);
            downLeftButtonSlot = new JoyButtonSlot(keyToKeycode("KP_1"), JoyButtonSlot::JoyKeyboard, this);
            downRightButtonSlot = new JoyButtonSlot(keyToKeycode("KP_3"), JoyButtonSlot::JoyKeyboard, this);
        }
    }
    else if (index == 8)
    {
        QHash<int, JoyDPadButton*> *buttons = dpad->getButtons();
        QHashIterator<int, JoyDPadButton*> iter(*buttons);
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            button->clearSlotsEventReset();
        }
    }

    if (upButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upButtonSlot->getSlotCode(), upButtonSlot->getSlotMode());
        upButtonSlot->deleteLater();
    }

    if (downButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downButtonSlot->getSlotCode(), downButtonSlot->getSlotMode());
        downButtonSlot->deleteLater();
    }

    if (leftButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadLeft);
        button->clearSlotsEventReset();
        button->setAssignedSlot(leftButtonSlot->getSlotCode(), leftButtonSlot->getSlotMode());
        leftButtonSlot->deleteLater();
    }

    if (rightButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadRight);
        button->clearSlotsEventReset();
        button->setAssignedSlot(rightButtonSlot->getSlotCode(), rightButtonSlot->getSlotMode());
        rightButtonSlot->deleteLater();
    }

    if (upLeftButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadLeftUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upLeftButtonSlot->getSlotCode(), upLeftButtonSlot->getSlotMode());
        upLeftButtonSlot->deleteLater();
    }

    if (upRightButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadRightUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upRightButtonSlot->getSlotCode(), upRightButtonSlot->getSlotMode());
        upRightButtonSlot->deleteLater();
    }

    if (downLeftButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadLeftDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downLeftButtonSlot->getSlotCode(), downLeftButtonSlot->getSlotMode());
        downLeftButtonSlot->deleteLater();
    }

    if (downRightButtonSlot)
    {
        JoyDPadButton *button = dpad->getJoyButton(JoyDPadButton::DpadRightDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downRightButtonSlot->getSlotCode(), downRightButtonSlot->getSlotMode());
        downRightButtonSlot->deleteLater();
    }
}

void DPadEditDialog::implementModes(int index)
{
    dpad->releaseButtonEvents();

    if (index == 0)
    {
        dpad->setJoyMode(JoyDPad::StandardMode);
    }
    else if (index == 1)
    {
        dpad->setJoyMode(JoyDPad::EightWayMode);
    }
}
