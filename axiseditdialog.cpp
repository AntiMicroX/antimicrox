#include <QDebug>

#include "axiseditdialog.h"
#include "ui_axiseditdialog.h"
#include "advancebuttondialog.h"
#include "buttoneditdialog.h"
#include "event.h"

AxisEditDialog::AxisEditDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    setAxisThrottleConfirm = new SetAxisThrottleDialog(axis, this);

    this->axis = axis;

    this->setWindowTitle(QString("Set Axis %1").arg(axis->getRealJoyIndex()));

    initialThrottleState = axis->getThrottle();

    ui->horizontalSlider->setValue(axis->getDeadZone());
    ui->lineEdit->setText(QString::number(axis->getDeadZone()));

    ui->horizontalSlider_2->setValue(axis->getMaxZoneValue());
    ui->lineEdit_2->setText(QString::number(axis->getMaxZoneValue()));

    JoyAxisButton *nButton = axis->getNAxisButton();

    ui->nPushButton->setText(nButton->getSlotsSummary());

    JoyAxisButton *pButton = axis->getPAxisButton();

    ui->pPushButton->setText(pButton->getSlotsSummary());

    int currentThrottle = axis->getThrottle();
    ui->comboBox_2->setCurrentIndex(currentThrottle+1);
    if (currentThrottle == -1)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
    }
    else if (currentThrottle == 1)
    {
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
    }

    ui->axisstatusBox->setDeadZone(axis->getDeadZone());
    ui->axisstatusBox->setMaxZone(axis->getMaxZoneValue());
    ui->axisstatusBox->setThrottle(axis->getThrottle());

    QString currentJoyValueText ("Current Value: ");
    currentJoyValueText = currentJoyValueText.append(QString::number(axis->getCurrentRawValue()));
    ui->joyValueLabel->setText(currentJoyValueText);
    ui->axisstatusBox->setValue(axis->getCurrentRawValue());

    if (pButton->getMouseSpeedX() == nButton->getMouseSpeedX())
    {
        ui->horizontalSpinBox->setValue(pButton->getMouseSpeedX());
        updateHorizontalSpeedConvertLabel(pButton->getMouseSpeedX());
    }
    else
    {
        int temp = (pButton->getMouseSpeedX() > nButton->getMouseSpeedX()) ? pButton->getMouseSpeedX() : nButton->getMouseSpeedX();
        ui->horizontalSpinBox->setValue(temp);
        updateHorizontalSpeedConvertLabel(temp);
    }

    if (pButton->getMouseSpeedY() == nButton->getMouseSpeedY())
    {
        ui->verticalSpinBox->setValue(nButton->getMouseSpeedY());
        updateVerticalSpeedConvertLabel(nButton->getMouseSpeedY());
    }
    else
    {
        int temp = (pButton->getMouseSpeedY() > nButton->getMouseSpeedY()) ? pButton->getMouseSpeedY() : nButton->getMouseSpeedY();
        ui->verticalSpinBox->setValue(temp);
        updateVerticalSpeedConvertLabel(temp);
    }

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));

    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDeadZoneBox(int)));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setDeadZone(int)));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), axis, SLOT(setDeadZone(int)));

    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(updateMaxZoneBox(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setMaxZone(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), axis, SLOT(setMaxZoneValue(int)));

    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateThrottleUi(int)));
    connect(axis, SIGNAL(moved(int)), ui->axisstatusBox, SLOT(setValue(int)));
    connect(axis, SIGNAL(moved(int)), this, SLOT(updateJoyValue(int)));

    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateDeadZoneSlider(QString)));
    connect(ui->lineEdit_2, SIGNAL(textEdited(QString)), this, SLOT(updateMaxZoneSlider(QString)));

    connect(ui->nPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedNDialog()));
    connect(ui->pPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedPDialog()));

    connect(ui->changeTogetherCheckBox, SIGNAL(clicked(bool)), this, SLOT(syncSpeedSpinBoxes()));
    connect(ui->changeMouseSpeedsCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeMouseSpeedsInterface(bool)));

    connect(this, SIGNAL(finished(int)), this, SLOT(checkFinalSettings()));
}

AxisEditDialog::~AxisEditDialog()
{
    delete ui;
}

void AxisEditDialog::implementPresets(int index)
{
    JoyButtonSlot *nbuttonslot = 0;
    JoyButtonSlot *pbuttonslot = 0;

    if (index == 1)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 2)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 3)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 4)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (index == 5)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("Up"), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("Down"), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 6)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("Left"), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("Right"), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 7)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("w"), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("s"), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 8)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("a"), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("d"), JoyButtonSlot::JoyKeyboard, this);
    }

    if (nbuttonslot)
    {
        JoyButton *button = axis->getNAxisButton();
        button->clearSlotsEventReset();
        button->setAssignedSlot(nbuttonslot->getSlotCode(), nbuttonslot->getSlotMode());
        refreshNButtonLabel();
        nbuttonslot->deleteLater();
    }

    if (pbuttonslot)
    {
        JoyButton *button = axis->getPAxisButton();
        button->clearSlotsEventReset();
        button->setAssignedSlot(pbuttonslot->getSlotCode(), pbuttonslot->getSlotMode());
        refreshPButtonLabel();
        pbuttonslot->deleteLater();
    }
}

void AxisEditDialog::updateDeadZoneBox(int value)
{
    ui->lineEdit->setText(QString::number(value));
}

void AxisEditDialog::updateMaxZoneBox(int value)
{
    ui->lineEdit_2->setText(QString::number(value));
}

void AxisEditDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void AxisEditDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->verticalSpeedLabel->setText(label);
}

void AxisEditDialog::updateThrottleUi(int index)
{
    if (index == 0)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
    }
    else if (index == 1)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(true);
    }
    else if (index == 2)
    {
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
    }

    ui->axisstatusBox->setThrottle(index - 1);
    axis->setThrottle(index - 1);
}

void AxisEditDialog::updateJoyValue(int index)
{
    QString currentJoyValueText ("Current Value: ");
    currentJoyValueText = currentJoyValueText.append(QString::number(index));
    ui->joyValueLabel->setText(currentJoyValueText);
}

void AxisEditDialog::updateDeadZoneSlider(QString value)
{
    int temp = value.toInt();
    if (temp >= JoyAxis::AXISMIN && temp <= JoyAxis::AXISMAX)
    {
        ui->horizontalSlider->setValue(temp);
    }
}

void AxisEditDialog::updateMaxZoneSlider(QString value)
{
    int temp = value.toInt();
    if (temp >= JoyAxis::AXISMIN && temp <= JoyAxis::AXISMAX)
    {
        ui->horizontalSlider_2->setValue(temp);
    }
}

void AxisEditDialog::openAdvancedPDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(axis->getPAxisButton());
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshPButtonLabel()));
}

void AxisEditDialog::openAdvancedNDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(axis->getNAxisButton());
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshNButtonLabel()));
}

void AxisEditDialog::refreshNButtonLabel()
{
    ui->nPushButton->setText(axis->getNAxisButton()->getSlotsSummary());
}

void AxisEditDialog::refreshPButtonLabel()
{
    ui->pPushButton->setText(axis->getPAxisButton()->getSlotsSummary());
}

void AxisEditDialog::syncSpeedSpinBoxes()
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

void AxisEditDialog::moveSpeedsTogether(int value)
{
    if (ui->changeTogetherCheckBox->isChecked())
    {
        ui->horizontalSpinBox->setValue(value);
        ui->verticalSpinBox->setValue(value);
    }
}

void AxisEditDialog::changeMouseSpeedsInterface(bool value)
{
    ui->horizontalSpinBox->setEnabled(value);
    ui->verticalSpinBox->setEnabled(value);
    ui->changeTogetherCheckBox->setEnabled(value);
}

void AxisEditDialog::updateConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        axis->getPAxisButton()->setMouseSpeedX(value);
        axis->getNAxisButton()->setMouseSpeedX(value);
    }
}

void AxisEditDialog::updateConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        axis->getPAxisButton()->setMouseSpeedY(value);
        axis->getNAxisButton()->setMouseSpeedY(value);
    }
}

void AxisEditDialog::checkFinalSettings()
{
    if (axis->getThrottle() != initialThrottleState)
    {
        setAxisThrottleConfirm->exec();
    }
}
