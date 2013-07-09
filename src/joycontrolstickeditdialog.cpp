#include <QHashIterator>

#include "joycontrolstickeditdialog.h"
#include "ui_joycontrolstickeditdialog.h"
#include "buttoneditdialog.h"
#include "event.h"

JoyControlStickEditDialog::JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::JoyControlStickEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->stick = stick;

    this->setWindowTitle(QString(tr("Set Stick %1")).arg(stick->getRealJoyIndex()));

    ui->deadZoneSlider->setValue(stick->getDeadZone());
    ui->deadZoneSpinBox->setValue(stick->getDeadZone());

    ui->maxZoneSlider->setValue(stick->getMaxZone());
    ui->maxZoneSpinBox->setValue(stick->getMaxZone());

    ui->diagonalRangeSlider->setValue(stick->getDiagonalRange());
    ui->diagonalRangeSpinBox->setValue(stick->getDiagonalRange());

    ui->xCoordinateLabel->setText(QString::number(stick->getXCoordinate()));
    ui->yCoordinateLabel->setText(QString::number(stick->getYCoordinate()));
    ui->distanceLabel->setText(QString::number(stick->getAbsoluteDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));

    if (stick->getJoyMode() == JoyControlStick::StandardMode)
    {
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (stick->getJoyMode() == JoyControlStick::EightWayMode)
    {
        ui->joyModeComboBox->setCurrentIndex(1);
    }

    ui->stickStatusBoxWidget->setStick(stick);

    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    int tempMouseSpeedX = 0;
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        tempMouseSpeedX = qMax(tempMouseSpeedX, button->getMouseSpeedX());
    }

    iter.toFront();
    int tempMouseSpeedY = 0;
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
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

    connect(ui->deadZoneSlider, SIGNAL(valueChanged(int)), ui->deadZoneSpinBox, SLOT(setValue(int)));
    connect(ui->maxZoneSlider, SIGNAL(valueChanged(int)), ui->maxZoneSpinBox, SLOT(setValue(int)));
    connect(ui->diagonalRangeSlider, SIGNAL(valueChanged(int)), ui->diagonalRangeSpinBox, SLOT(setValue(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), ui->deadZoneSlider, SLOT(setValue(int)));
    connect(ui->maxZoneSpinBox, SIGNAL(valueChanged(int)), ui->maxZoneSlider, SLOT(setValue(int)));
    connect(ui->maxZoneSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkMaxZone(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), ui->diagonalRangeSlider, SLOT(setValue(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDeadZone(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDiagonalRange(int)));

    connect(ui->changeTogetherCheckBox, SIGNAL(clicked(bool)), this, SLOT(syncSpeedSpinBoxes()));
    connect(ui->changeMouseSpeedsCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeMouseSpeedsInterface(bool)));

    connect(stick, SIGNAL(moved(int,int)), this, SLOT(refreshStickStats(int,int)));
}

JoyControlStickEditDialog::~JoyControlStickEditDialog()
{
    delete ui;
}

void JoyControlStickEditDialog::implementPresets(int index)
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
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *buttons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*buttons);
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->clearSlotsEventReset();
        }
    }

    if (upButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upButtonSlot->getSlotCode(), upButtonSlot->getSlotMode());
        upButtonSlot->deleteLater();
    }

    if (downButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downButtonSlot->getSlotCode(), downButtonSlot->getSlotMode());
        downButtonSlot->deleteLater();
    }

    if (leftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeft);
        button->clearSlotsEventReset();
        button->setAssignedSlot(leftButtonSlot->getSlotCode(), leftButtonSlot->getSlotMode());
        leftButtonSlot->deleteLater();
    }

    if (rightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRight);
        button->clearSlotsEventReset();
        button->setAssignedSlot(rightButtonSlot->getSlotCode(), rightButtonSlot->getSlotMode());
        rightButtonSlot->deleteLater();
    }

    if (upLeftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeftUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upLeftButtonSlot->getSlotCode(), upLeftButtonSlot->getSlotMode());
        upLeftButtonSlot->deleteLater();
    }

    if (upRightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRightUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upRightButtonSlot->getSlotCode(), upRightButtonSlot->getSlotMode());
        upRightButtonSlot->deleteLater();
    }

    if (downLeftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeftDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downLeftButtonSlot->getSlotCode(), downLeftButtonSlot->getSlotMode());
        downLeftButtonSlot->deleteLater();
    }

    if (downRightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRightDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downRightButtonSlot->getSlotCode(), downRightButtonSlot->getSlotMode());
        downRightButtonSlot->deleteLater();
    }
}

void JoyControlStickEditDialog::refreshStickStats(int x, int y)
{
    ui->xCoordinateLabel->setText(QString::number(x));
    ui->yCoordinateLabel->setText(QString::number(y));
    ui->distanceLabel->setText(QString::number(stick->getAbsoluteDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));
}

void JoyControlStickEditDialog::syncSpeedSpinBoxes()
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

void JoyControlStickEditDialog::changeMouseSpeedsInterface(bool value)
{
    ui->horizontalSpinBox->setEnabled(value);
    ui->verticalSpinBox->setEnabled(value);
    ui->changeTogetherCheckBox->setEnabled(value);
}

void JoyControlStickEditDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void JoyControlStickEditDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->verticalSpeedLabel->setText(label);
}

void JoyControlStickEditDialog::moveSpeedsTogether(int value)
{
    if (ui->changeTogetherCheckBox->isChecked())
    {
        ui->horizontalSpinBox->setValue(value);
        ui->verticalSpinBox->setValue(value);
    }
}

void JoyControlStickEditDialog::updateConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->setMouseSpeedX(value);
        }
    }
}

void JoyControlStickEditDialog::updateConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->setMouseSpeedY(value);
        }
    }
}

void JoyControlStickEditDialog::checkMaxZone(int value)
{
    if (value > ui->deadZoneSpinBox->value())
    {
        stick->setMaxZone(value);
    }
}

void JoyControlStickEditDialog::implementModes(int index)
{
    stick->releaseButtonEvents();

    if (index == 0)
    {
        stick->setJoyMode(JoyControlStick::StandardMode);
    }
    else if (index == 1)
    {
        stick->setJoyMode(JoyControlStick::EightWayMode);
    }
}
