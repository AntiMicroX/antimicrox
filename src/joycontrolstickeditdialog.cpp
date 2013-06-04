#include "joycontrolstickeditdialog.h"
#include "ui_joycontrolstickeditdialog.h"
#include "buttoneditdialog.h"
#include "event.h"

JoyControlStickEditDialog::JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JoyControlStickEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->stick = stick;

    this->setWindowTitle(QString(tr("Set Stick %1")).arg(stick->getRealJoyIndex()));

    ui->deadZoneSlider->setValue(stick->getDeadZone());
    ui->deadZoneSpinBox->setValue(stick->getDeadZone());

    ui->diagonalRangeSlider->setValue(stick->getDiagonalRange());
    ui->diagonalRangeSpinBox->setValue(stick->getDiagonalRange());

    ui->upPushButton->setButton(stick->getDirectionButton(JoyControlStick::StickUp));
    ui->upPushButton->setText(stick->getDirectionButton(JoyControlStick::StickUp)->getSlotsSummary());

    ui->downPushButton->setButton(stick->getDirectionButton(JoyControlStick::StickDown));
    ui->downPushButton->setText(stick->getDirectionButton(JoyControlStick::StickDown)->getSlotsSummary());

    ui->leftPushButton->setButton(stick->getDirectionButton(JoyControlStick::StickLeft));
    ui->leftPushButton->setText(stick->getDirectionButton(JoyControlStick::StickLeft)->getSlotsSummary());

    ui->rightPushButton->setButton(stick->getDirectionButton(JoyControlStick::StickRight));
    ui->rightPushButton->setText(stick->getDirectionButton(JoyControlStick::StickRight)->getSlotsSummary());

    ui->xCoordinateLabel->setText(QString::number(stick->getXCoordinate()));
    ui->yCoordinateLabel->setText(QString::number(stick->getYCoordinate()));
    ui->distanceLabel->setText(QString::number(stick->getAbsoluteDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));

    ui->stickStatusBoxWidget->setStick(stick);

    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
    connect(ui->deadZoneSlider, SIGNAL(valueChanged(int)), ui->deadZoneSpinBox, SLOT(setValue(int)));
    connect(ui->diagonalRangeSlider, SIGNAL(valueChanged(int)), ui->diagonalRangeSpinBox, SLOT(setValue(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), ui->deadZoneSlider, SLOT(setValue(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), ui->diagonalRangeSlider, SLOT(setValue(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDeadZone(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDiagonalRange(int)));

    connect(ui->upPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedUpDialog()));
    connect(ui->downPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedDownDialog()));
    connect(ui->leftPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedLeftDialog()));
    connect(ui->rightPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedRightDialog()));

    connect(stick, SIGNAL(moved(int,int)), this, SLOT(refreshStickStats(int,int)));
}

JoyControlStickEditDialog::~JoyControlStickEditDialog()
{
    delete ui;
}

void JoyControlStickEditDialog::openAdvancedUpDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(stick->getDirectionButton(JoyControlStick::StickUp), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshUpButtonLabel()));
}

void JoyControlStickEditDialog::openAdvancedDownDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(stick->getDirectionButton(JoyControlStick::StickDown), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshDownButtonLabel()));
}

void JoyControlStickEditDialog::openAdvancedLeftDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(stick->getDirectionButton(JoyControlStick::StickLeft), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshLeftButtonLabel()));
}

void JoyControlStickEditDialog::openAdvancedRightDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(stick->getDirectionButton(JoyControlStick::StickRight), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshRightButtonLabel()));
}

void JoyControlStickEditDialog::refreshUpButtonLabel()
{
    ui->upPushButton->setText(stick->getDirectionButton(JoyControlStick::StickUp)->getSlotsSummary());
}

void JoyControlStickEditDialog::refreshDownButtonLabel()
{
    ui->downPushButton->setText(stick->getDirectionButton(JoyControlStick::StickDown)->getSlotsSummary());
}

void JoyControlStickEditDialog::refreshLeftButtonLabel()
{
    ui->leftPushButton->setText(stick->getDirectionButton(JoyControlStick::StickLeft)->getSlotsSummary());
}

void JoyControlStickEditDialog::refreshRightButtonLabel()
{
    ui->rightPushButton->setText(stick->getDirectionButton(JoyControlStick::StickRight)->getSlotsSummary());
}

void JoyControlStickEditDialog::implementPresets(int index)
{
    JoyButtonSlot *upButtonSlot = 0;
    JoyButtonSlot *downButtonSlot = 0;
    JoyButtonSlot *leftButtonSlot = 0;
    JoyButtonSlot *rightButtonSlot = 0;

    if (index == 1)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 2)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 3)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 4)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 5)
    {
        upButtonSlot = new JoyButtonSlot(keyToKeycode("Up"), JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(keyToKeycode("Down"), JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(keyToKeycode("Left"), JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(keyToKeycode("Right"), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 6)
    {
        upButtonSlot = new JoyButtonSlot(keyToKeycode("w"), JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(keyToKeycode("s"), JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(keyToKeycode("a"), JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(keyToKeycode("d"), JoyButtonSlot::JoyKeyboard, this);
    }

    if (upButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickUp);
        button->clearSlotsEventReset();
        button->setAssignedSlot(upButtonSlot->getSlotCode(), upButtonSlot->getSlotMode());
        refreshUpButtonLabel();
        upButtonSlot->deleteLater();
    }

    if (downButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickDown);
        button->clearSlotsEventReset();
        button->setAssignedSlot(downButtonSlot->getSlotCode(), downButtonSlot->getSlotMode());
        refreshDownButtonLabel();
        downButtonSlot->deleteLater();
    }

    if (leftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeft);
        button->clearSlotsEventReset();
        button->setAssignedSlot(leftButtonSlot->getSlotCode(), leftButtonSlot->getSlotMode());
        refreshLeftButtonLabel();
        leftButtonSlot->deleteLater();
    }

    if (rightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRight);
        button->clearSlotsEventReset();
        button->setAssignedSlot(rightButtonSlot->getSlotCode(), rightButtonSlot->getSlotMode());
        refreshRightButtonLabel();
        rightButtonSlot->deleteLater();
    }
}

void JoyControlStickEditDialog::refreshStickStats(int x, int y)
{
    ui->xCoordinateLabel->setText(QString::number(x));
    ui->yCoordinateLabel->setText(QString::number(y));
    ui->distanceLabel->setText(QString::number(stick->getAbsoluteDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));
}
