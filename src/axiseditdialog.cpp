#include <QDebug>
#include <QList>

#include "axiseditdialog.h"
#include "ui_axiseditdialog.h"
#include "buttoneditdialog.h"
#include "mousedialog/mouseaxissettingsdialog.h"
#include "event.h"
#include "antkeymapper.h"

AxisEditDialog::AxisEditDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    setAxisThrottleConfirm = new SetAxisThrottleDialog(axis, this);

    this->axis = axis;

    updateWindowTitleAxisName();

    initialThrottleState = axis->getThrottle();

    ui->horizontalSlider->setValue(axis->getDeadZone());
    ui->lineEdit->setText(QString::number(axis->getDeadZone()));

    ui->horizontalSlider_2->setValue(axis->getMaxZoneValue());
    ui->lineEdit_2->setText(QString::number(axis->getMaxZoneValue()));

    JoyAxisButton *nButton = axis->getNAxisButton();

    if (!nButton->getActionName().isEmpty())
    {
        ui->nPushButton->setText(nButton->getActionName());
    }
    else
    {
        ui->nPushButton->setText(nButton->getSlotsSummary());
    }

    JoyAxisButton *pButton = axis->getPAxisButton();

    if (!pButton->getActionName().isEmpty())
    {
        ui->pPushButton->setText(pButton->getActionName());
    }
    else
    {
        ui->pPushButton->setText(pButton->getSlotsSummary());
    }

    int currentThrottle = axis->getThrottle();
    //ui->comboBox_2->setCurrentIndex(currentThrottle+1);
    if (currentThrottle == JoyAxis::NegativeThrottle || currentThrottle == JoyAxis::NegativeHalfThrottle)
    {
        int tempindex = currentThrottle == JoyAxis::NegativeHalfThrottle ? 0 : 1;
        ui->comboBox_2->setCurrentIndex(tempindex);
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
    }
    else if (currentThrottle == JoyAxis::PositiveThrottle || currentThrottle == JoyAxis::PositiveHalfThrottle)
    {
        int tempindex = currentThrottle == JoyAxis::PositiveThrottle ? 3 : 4;
        ui->comboBox_2->setCurrentIndex(tempindex);
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
    }

    ui->axisstatusBox->setDeadZone(axis->getDeadZone());
    ui->axisstatusBox->setMaxZone(axis->getMaxZoneValue());
    ui->axisstatusBox->setThrottle(axis->getThrottle());

    ui->joyValueLabel->setText(QString::number(axis->getCurrentRawValue()));
    ui->axisstatusBox->setValue(axis->getCurrentRawValue());

    selectCurrentPreset();

    ui->axisNameLineEdit->setText(axis->getAxisName());

    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));

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

    connect(ui->mouseSettingsPushButton, SIGNAL(clicked()), this, SLOT(openMouseSettingsDialog()));
    connect(ui->axisNameLineEdit, SIGNAL(textEdited(QString)), axis, SLOT(setAxisName(QString)));

    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(updateWindowTitleAxisName()));
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
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_Up), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_Down), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 6)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_Left), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_Right), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 7)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_W), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_S), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 8)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_A), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(Qt::Key_D), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 9)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_8), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_2), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 10)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_4), JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_6), JoyButtonSlot::JoyKeyboard, this);
    }
    else if (index == 11)
    {
        JoyAxisButton *nbutton = axis->getNAxisButton();
        JoyAxisButton *pbutton = axis->getPAxisButton();
        nbutton->clearSlotsEventReset();
        refreshNButtonLabel();

        pbutton->clearSlotsEventReset();
        refreshPButtonLabel();
    }

    if (nbuttonslot)
    {
        JoyAxisButton *button = axis->getNAxisButton();
        button->clearSlotsEventReset();
        button->setAssignedSlot(nbuttonslot->getSlotCode(), nbuttonslot->getSlotMode());
        refreshNButtonLabel();
        nbuttonslot->deleteLater();
    }

    if (pbuttonslot)
    {
        JoyAxisButton *button = axis->getPAxisButton();
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

void AxisEditDialog::updateThrottleUi(int index)
{
    int tempthrottle = 0;
    if (index == 0 || index == 1)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
        tempthrottle = index == 0 ? JoyAxis::NegativeHalfThrottle : JoyAxis::NegativeThrottle;
    }
    else if (index == 2)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(true);
        tempthrottle = JoyAxis::NormalThrottle;
    }
    else if (index == 3 || index == 4)
    {
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
        tempthrottle = index == 3 ? JoyAxis::PositiveThrottle : JoyAxis::PositiveHalfThrottle;
    }

    axis->setThrottle(tempthrottle);
    ui->axisstatusBox->setThrottle(tempthrottle);
}

void AxisEditDialog::updateJoyValue(int value)
{
    ui->joyValueLabel->setText(QString::number(value));
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
    ButtonEditDialog *dialog = new ButtonEditDialog(axis->getPAxisButton(), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshPButtonLabel()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshPreset()));
}

void AxisEditDialog::openAdvancedNDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(axis->getNAxisButton(), this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshNButtonLabel()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(refreshPreset()));
}

void AxisEditDialog::refreshNButtonLabel()
{
    /*if (!axis->getNAxisButton()->getActionName().isEmpty())
    {
        ui->nPushButton->setText(axis->getNAxisButton()->getActionName());
    }
    else
    {
        ui->nPushButton->setText(axis->getNAxisButton()->getSlotsSummary());
    }*/
    ui->nPushButton->setText(axis->getNAxisButton()->getSlotsSummary());
}

void AxisEditDialog::refreshPButtonLabel()
{
    /*if (!axis->getPAxisButton()->getActionName().isEmpty())
    {
        ui->pPushButton->setText(axis->getPAxisButton()->getActionName());
    }
    else
    {
        ui->pPushButton->setText(axis->getPAxisButton()->getSlotsSummary());
    }*/
    ui->pPushButton->setText(axis->getPAxisButton()->getSlotsSummary());

}

void AxisEditDialog::checkFinalSettings()
{
    if (axis->getThrottle() != initialThrottleState)
    {
        setAxisThrottleConfirm->exec();
    }
}

void AxisEditDialog::selectCurrentPreset()
{
    JoyAxisButton *naxisbutton = axis->getNAxisButton();
    QList<JoyButtonSlot*> *naxisslots = naxisbutton->getAssignedSlots();
    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    QList<JoyButtonSlot*> *paxisslots = paxisbutton->getAssignedSlots();

    if (naxisslots->length() == 1 && paxisslots->length() == 1)
    {
        JoyButtonSlot *nslot = naxisslots->at(0);
        JoyButtonSlot *pslot = paxisslots->at(0);
        if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseLeft &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseRight)
        {
            ui->presetsComboBox->setCurrentIndex(1);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseRight &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseLeft)
        {
            ui->presetsComboBox->setCurrentIndex(2);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseUp &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseDown)
        {
            ui->presetsComboBox->setCurrentIndex(3);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseDown &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseUp)
        {
            ui->presetsComboBox->setCurrentIndex(4);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_Up) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_Down))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_Left) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_Right))
        {
            ui->presetsComboBox->setCurrentIndex(6);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_W) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_S))
        {
            ui->presetsComboBox->setCurrentIndex(7);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_A) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(Qt::Key_D))
        {
            ui->presetsComboBox->setCurrentIndex(8);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_8) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_2))
        {
            ui->presetsComboBox->setCurrentIndex(9);
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_4) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::returnVirtualKey(QtKeyMapperBase::AntKey_KP_6))
        {
            ui->presetsComboBox->setCurrentIndex(10);
        }
        else
        {
            ui->presetsComboBox->setCurrentIndex(0);
        }
    }
    else if (naxisslots->length() == 0 && paxisslots->length() == 0)
    {
        ui->presetsComboBox->setCurrentIndex(11);
    }
    else
    {
        ui->presetsComboBox->setCurrentIndex(0);
    }
}

void AxisEditDialog::refreshPreset()
{
    // Disconnect event associated with presetsComboBox so a change in the index does not
    // alter the axis buttons
    disconnect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
    selectCurrentPreset();
    // Reconnect the event
    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
}

void AxisEditDialog::openMouseSettingsDialog()
{
    ui->mouseSettingsPushButton->setEnabled(false);

    MouseAxisSettingsDialog *dialog = new MouseAxisSettingsDialog(this->axis, this);
    dialog->show();
    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(enableMouseSettingButton()));
}

void AxisEditDialog::enableMouseSettingButton()
{
    ui->mouseSettingsPushButton->setEnabled(true);
}

void AxisEditDialog::updateWindowTitleAxisName()
{
    QString temp = QString(tr("Set")).append(" ");
    if (!axis->getAxisName().isEmpty())
    {
        temp.append(tr("Axis")).append(" ").append(axis->getAxisName());
    }
    else
    {
        temp.append(tr("Axis")).append(" ").append(QString::number(axis->getRealJoyIndex()));
    }
    setWindowTitle(temp);
}
