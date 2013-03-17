#include <QDebug>

#include "axiseditdialog.h"
#include "ui_axiseditdialog.h"
#include "advancebuttondialog.h"
#include "event.h"

AxisEditDialog::AxisEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    axis = 0;
}

AxisEditDialog::AxisEditDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->axis = axis;

    this->setWindowTitle(QString("Set Axis %1").arg(axis->getRealJoyIndex()));

    ui->horizontalSlider->setValue(axis->getDeadZone());
    ui->lineEdit->setText(QString::number(axis->getDeadZone()));

    ui->horizontalSlider_2->setValue(axis->getMaxZoneValue());
    ui->lineEdit_2->setText(QString::number(axis->getMaxZoneValue()));

    JoyAxisButton *button = axis->getNAxisButton();
    tempNConfig = new ButtonTempConfig(button);

    QListIterator<JoyButtonSlot*> iter(*(button->getAssignedSlots()));
    while (iter.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter.next();
        tempNConfig->assignments->append(tempbuttonslot);
    }

    ui->pushButton->setText(axis->getNAxisButton()->getSlotsSummary());

    button = axis->getPAxisButton();
    tempPConfig = new ButtonTempConfig(button);

    QListIterator<JoyButtonSlot*> iter2(*(button->getAssignedSlots()));
    while (iter2.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter2.next();
        tempPConfig->assignments->append(tempbuttonslot);
    }

    ui->pushButton_2->setText(axis->getPAxisButton()->getSlotsSummary());

    int currentThrottle = axis->getThrottle();
    ui->comboBox_2->setCurrentIndex(currentThrottle+1);
    if (currentThrottle == -1)
    {
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
    }
    else if (currentThrottle == 1)
    {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
    }

    ui->axisstatusBox->setDeadZone(axis->getDeadZone());
    ui->axisstatusBox->setMaxZone(axis->getMaxZoneValue());
    ui->axisstatusBox->setThrottle(axis->getThrottle());

    QString currentJoyValueText ("Current Value: ");
    currentJoyValueText = currentJoyValueText.append(QString::number(axis->getCurrentRawValue()));
    ui->joyValueLabel->setText(currentJoyValueText);

    if (tempPConfig->mouseSpeedX == tempNConfig->mouseSpeedX)
    {
        ui->horizontalSpinBox->setValue(tempPConfig->mouseSpeedX);
        updateHorizontalSpeedConvertLabel(tempPConfig->mouseSpeedX);
    }
    else
    {
        int temp = (tempPConfig->mouseSpeedX > tempNConfig->mouseSpeedX) ? tempPConfig->mouseSpeedX : tempNConfig->mouseSpeedX;
        ui->horizontalSpinBox->setValue(temp);
        updateHorizontalSpeedConvertLabel(temp);
    }

    if (tempPConfig->mouseSpeedY == tempNConfig->mouseSpeedY)
    {
        ui->verticalSpinBox->setValue(tempNConfig->mouseSpeedY);
        updateVerticalSpeedConvertLabel(tempNConfig->mouseSpeedY);
    }
    else
    {
        int temp = (tempPConfig->mouseSpeedY > tempNConfig->mouseSpeedY) ? tempPConfig->mouseSpeedY : tempNConfig->mouseSpeedY;
        ui->verticalSpinBox->setValue(temp);
        updateVerticalSpeedConvertLabel(temp);
    }

    connect(this, SIGNAL(accepted()), this, SLOT(saveAxisChanges()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTempConfigHorizontalSpeed(int)));

    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTempConfigVerticalSpeed(int)));

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDeadZoneBox(int)));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setDeadZone(int)));

    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(updateMaxZoneBox(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setMaxZone(int)));

    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateThrottleUi(int)));
    connect(axis, SIGNAL(moved(int)), ui->axisstatusBox, SLOT(setValue(int)));
    connect(axis, SIGNAL(moved(int)), this, SLOT(updateJoyValue(int)));

    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateDeadZoneSlider(QString)));
    connect(ui->lineEdit_2, SIGNAL(textEdited(QString)), this, SLOT(updateMaxZoneSlider(QString)));

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(openAdvancedNDialog()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(openAdvancedPDialog()));

    connect(ui->changeTogetherCheckBox, SIGNAL(clicked(bool)), this, SLOT(syncSpeedSpinBoxes()));
    connect(ui->changeMouseSpeedsCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeMouseSpeedsInterface(bool)));
}

AxisEditDialog::~AxisEditDialog()
{
    delete ui;
    delete tempPConfig;
    delete tempNConfig;
}

void AxisEditDialog::saveAxisChanges()
{
    JoyAxisButton *naxisbutton = axis->getNAxisButton();
    JoyAxisButton *paxisbutton = axis->getPAxisButton();

    naxisbutton->reset();
    QListIterator<JoyButtonSlot*> iter(*(tempNConfig->assignments));
    while (iter.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter.next();
        naxisbutton->setAssignedSlot(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
    }

    paxisbutton->reset();
    QListIterator<JoyButtonSlot*> iter2(*(tempPConfig->assignments));
    while (iter2.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter2.next();
        paxisbutton->setAssignedSlot(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
    }

    axis->setDeadZone(ui->horizontalSlider->value());
    axis->setMaxZoneValue(ui->horizontalSlider_2->value());

    naxisbutton->setMouseSpeedX(tempNConfig->mouseSpeedX);
    naxisbutton->setMouseSpeedY(tempNConfig->mouseSpeedY);

    paxisbutton->setMouseSpeedX(tempPConfig->mouseSpeedX);
    paxisbutton->setMouseSpeedY(tempPConfig->mouseSpeedY);

    int currentThrottle = 0;
    if (ui->comboBox_2->isEnabled())
    {
        currentThrottle = ui->comboBox_2->currentIndex() - 1;
    }
    axis->setThrottle(currentThrottle);

    if (tempPConfig->toggle)
    {
        paxisbutton->setToggle(true);
    }

    if (tempNConfig->toggle)
    {
        naxisbutton->setToggle(true);
    }
    //axis->joyEvent(axis->getCurrentThrottledDeadValue(), true);
}

void AxisEditDialog::implementPresets(int index)
{
    JoyButtonSlot *nbuttonslot = 0;
    JoyButtonSlot *pbuttonslot = 0;

    if (index == 1)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement);
    }
    else if (index == 2)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement);
    }
    else if (index == 3)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement);
    }
    else if (index == 4)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement);
    }
    else if (index == 5)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("Up"), JoyButtonSlot::JoyKeyboard);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("Down"), JoyButtonSlot::JoyKeyboard);
    }
    else if (index == 6)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("Left"), JoyButtonSlot::JoyKeyboard);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("Right"), JoyButtonSlot::JoyKeyboard);
    }
    else if (index == 7)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("w"), JoyButtonSlot::JoyKeyboard);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("s"), JoyButtonSlot::JoyKeyboard);
    }
    else if (index == 8)
    {
        nbuttonslot = new JoyButtonSlot(keyToKeycode("a"), JoyButtonSlot::JoyKeyboard);
        pbuttonslot = new JoyButtonSlot(keyToKeycode("d"), JoyButtonSlot::JoyKeyboard);
    }

    if (nbuttonslot)
    {
        tempNConfig->assignments->clear();
        tempNConfig->assignments->append(nbuttonslot);
        updateFromTempNConfig();
    }

    if (pbuttonslot)
    {
        tempPConfig->assignments->clear();
        tempPConfig->assignments->append(pbuttonslot);
        updateFromTempPConfig();
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
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
    }
    else if (index == 1)
    {
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    }
    else if (index == 2)
    {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
    }
    ui->axisstatusBox->setThrottle(index - 1);
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
    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(tempPConfig);
    dialog->show();

    connect(dialog, SIGNAL(accepted()), this, SLOT(updateFromTempPConfig()));
}

void AxisEditDialog::openAdvancedNDialog()
{
    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(tempNConfig);
    dialog->show();

    connect(dialog, SIGNAL(accepted()), this, SLOT(updateFromTempNConfig()));
}

void AxisEditDialog::updateFromTempPConfig()
{
    ui->pushButton_2->setText(tempPConfig->getSlotsSummary());
    if (tempPConfig->mouseSpeedX != ui->horizontalSpinBox->value())
    {
        ui->changeMouseSpeedsCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setEnabled(false);
        ui->horizontalSpinBox->setEnabled(false);
        ui->verticalSpinBox->setEnabled(false);
    }
    else if (tempPConfig->mouseSpeedY != ui->verticalSpinBox->value())
    {
        ui->changeMouseSpeedsCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setEnabled(false);
        ui->horizontalSpinBox->setEnabled(false);
        ui->verticalSpinBox->setEnabled(false);
    }

    if (tempPConfig->mouseSpeedX > ui->horizontalSpinBox->value())
    {
        ui->horizontalSpinBox->setValue(tempPConfig->mouseSpeedX);
    }

    if (tempPConfig->mouseSpeedY > ui->verticalSpinBox->value())
    {
        ui->verticalSpinBox->setValue(tempPConfig->mouseSpeedY);
    }
}

void AxisEditDialog::updateFromTempNConfig()
{
    ui->pushButton->setText(tempNConfig->getSlotsSummary());
    if (tempNConfig->mouseSpeedX != ui->horizontalSpinBox->value())
    {
        ui->changeMouseSpeedsCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setEnabled(false);
        ui->horizontalSpinBox->setEnabled(false);
        ui->verticalSpinBox->setEnabled(false);
    }
    else if (tempNConfig->mouseSpeedY != ui->verticalSpinBox->value())
    {
        ui->changeMouseSpeedsCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setChecked(false);
        ui->changeTogetherCheckBox->setEnabled(false);
        ui->horizontalSpinBox->setEnabled(false);
        ui->verticalSpinBox->setEnabled(false);
    }

    if (tempNConfig->mouseSpeedX > ui->horizontalSpinBox->value())
    {
        ui->horizontalSpinBox->setValue(tempNConfig->mouseSpeedX);
    }

    if (tempNConfig->mouseSpeedY > ui->verticalSpinBox->value())
    {
        ui->verticalSpinBox->setValue(tempNConfig->mouseSpeedY);
    }
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

void AxisEditDialog::updateTempConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        tempPConfig->mouseSpeedX = value;
        tempNConfig->mouseSpeedX = value;
    }
}

void AxisEditDialog::updateTempConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        tempPConfig->mouseSpeedY = value;
        tempNConfig->mouseSpeedY = value;
    }
}
