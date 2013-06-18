#include <QString>
#include <QListIterator>
#include <QHashIterator>

#include "advancestickassignmentdialog.h"
#include "ui_advancestickassignmentdialog.h"
#include "joycontrolstick.h"

AdvanceStickAssignmentDialog::AdvanceStickAssignmentDialog(Joystick *joystick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AdvanceStickAssignmentDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;

    QString tempHeaderLabel = ui->joystickNumberLabel->text();
    tempHeaderLabel = tempHeaderLabel.arg(joystick->getRealJoyNumber());
    ui->joystickNumberLabel->setText(tempHeaderLabel);

    ui->xAxisOneComboBox->addItem("", QVariant(0));
    ui->yAxisOneComboBox->addItem("", QVariant(0));

    ui->xAxisTwoComboBox->addItem("", QVariant(0));
    ui->yAxisTwoComboBox->addItem("", QVariant(0));

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        ui->xAxisOneComboBox->addItem(tr("Axis %1").arg(i+1), QVariant(i));
        ui->yAxisOneComboBox->addItem(tr("Axis %1").arg(i+1), QVariant(i));

        ui->xAxisTwoComboBox->addItem(tr("Axis %1").arg(i+1), QVariant(i));
        ui->yAxisTwoComboBox->addItem(tr("Axis %1").arg(i+1), QVariant(i));
    }

    refreshStickConfiguration();

    connect(ui->enableOneCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeStateStickOneWidgets(bool)));
    connect(ui->enableTwoCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeStateStickTwoWidgets(bool)));

    connect(ui->xAxisOneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickOne()));
    connect(ui->yAxisOneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickOne()));

    connect(ui->xAxisTwoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickTwo()));
    connect(ui->yAxisTwoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickTwo()));
}

AdvanceStickAssignmentDialog::~AdvanceStickAssignmentDialog()
{
    delete ui;
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickOne()
{
    if (ui->xAxisOneComboBox->currentIndex() > 0 && ui->yAxisOneComboBox->currentIndex() > 0)
    {
        if (ui->xAxisOneComboBox->currentIndex() != ui->yAxisOneComboBox->currentIndex())
        {
            for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                JoyAxis *axis1 = currentset->getJoyAxis(ui->xAxisOneComboBox->currentIndex()-1);
                JoyAxis *axis2 = currentset->getJoyAxis(ui->yAxisOneComboBox->currentIndex()-1);
                if (axis1 && axis2)
                {
                    JoyControlStick *controlstick = currentset->getJoyStick(0);
                    if (controlstick)
                    {
                        controlstick->replaceXAxis(axis1);
                        controlstick->replaceYAxis(axis2);
                    }
                    else
                    {
                        JoyControlStick *controlstick = new JoyControlStick(axis1, axis2, 0, i, currentset);
                        currentset->addControlStick(0, controlstick);
                    }
                }
            }

            refreshStickConfiguration();
        }
        else
        {
            if (sender() == ui->xAxisOneComboBox)
            {
                ui->yAxisOneComboBox->setCurrentIndex(0);
            }
            else if (sender() == ui->yAxisOneComboBox)
            {
                ui->xAxisOneComboBox->setCurrentIndex(0);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickTwo()
{
    if (ui->xAxisTwoComboBox->currentIndex() > 0 && ui->yAxisTwoComboBox->currentIndex() > 0)
    {
        if (ui->xAxisTwoComboBox->currentIndex() != ui->yAxisTwoComboBox->currentIndex())
        {
            for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                JoyAxis *axis1 = currentset->getJoyAxis(ui->xAxisTwoComboBox->currentIndex()-1);
                JoyAxis *axis2 = currentset->getJoyAxis(ui->yAxisTwoComboBox->currentIndex()-1);
                if (axis1 && axis2)
                {
                    JoyControlStick *controlstick = currentset->getJoyStick(1);
                    if (controlstick)
                    {
                        controlstick->replaceXAxis(axis1);
                        controlstick->replaceYAxis(axis2);
                    }
                    else
                    {
                        JoyControlStick *controlstick = new JoyControlStick(axis1, axis2, 1, i, currentset);
                        currentset->addControlStick(1, controlstick);
                    }
                }
            }

            refreshStickConfiguration();
        }
        else
        {
            if (sender() == ui->xAxisTwoComboBox)
            {
                ui->yAxisTwoComboBox->setCurrentIndex(0);
            }
            else if (sender() == ui->yAxisTwoComboBox)
            {
                ui->xAxisTwoComboBox->setCurrentIndex(0);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateStickOneWidgets(bool enabled)
{
    if (enabled)
    {
        ui->xAxisOneComboBox->setEnabled(true);
        ui->yAxisOneComboBox->setEnabled(true);
        ui->enableTwoCheckBox->setEnabled(true);
    }
    else
    {
        ui->xAxisOneComboBox->setEnabled(false);
        ui->xAxisOneComboBox->setCurrentIndex(0);
        ui->yAxisOneComboBox->setEnabled(false);
        ui->yAxisOneComboBox->setCurrentIndex(0);

        ui->xAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setCurrentIndex(0);
        ui->enableTwoCheckBox->setEnabled(false);
        ui->enableTwoCheckBox->setChecked(false);

        JoyControlStick *controlstick = joystick->getActiveSetJoystick()->getJoyStick(0);
        if (controlstick)
        {
            for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                currentset->removeControlStick(0);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateStickTwoWidgets(bool enabled)
{
    if (enabled)
    {
        ui->xAxisTwoComboBox->setEnabled(true);
        ui->yAxisTwoComboBox->setEnabled(true);
    }
    else
    {
        ui->xAxisTwoComboBox->setEnabled(false);
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setCurrentIndex(0);

        JoyControlStick *controlstick = joystick->getActiveSetJoystick()->getJoyStick(1);
        if (controlstick)
        {
            for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                currentset->removeControlStick(1);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::refreshStickConfiguration()
{
    JoyControlStick *stick1 = joystick->getActiveSetJoystick()->getJoyStick(0);
    JoyControlStick *stick2 = joystick->getActiveSetJoystick()->getJoyStick(1);
    if (stick1)
    {
        JoyAxis *axisX = stick1->getAxisX();
        JoyAxis *axisY = stick1->getAxisY();
        if (axisX && axisY)
        {
            ui->xAxisOneComboBox->setCurrentIndex(axisX->getRealJoyIndex());
            ui->yAxisOneComboBox->setCurrentIndex(axisY->getRealJoyIndex());
            ui->xAxisOneComboBox->setEnabled(true);
            ui->yAxisOneComboBox->setEnabled(true);
            ui->enableOneCheckBox->setEnabled(true);
            ui->enableOneCheckBox->setChecked(true);
            ui->enableTwoCheckBox->setEnabled(true);
        }
    }
    else
    {
        ui->xAxisOneComboBox->setCurrentIndex(0);
        ui->xAxisOneComboBox->setEnabled(false);
        ui->yAxisOneComboBox->setCurrentIndex(0);
        ui->yAxisOneComboBox->setEnabled(false);
        ui->enableOneCheckBox->setChecked(false);
        ui->enableTwoCheckBox->setEnabled(false);
    }

    if (stick2)
    {
        JoyAxis *axisX = stick2->getAxisX();
        JoyAxis *axisY = stick2->getAxisY();
        if (axisX && axisY)
        {
            ui->xAxisTwoComboBox->setCurrentIndex(axisX->getRealJoyIndex());
            ui->yAxisTwoComboBox->setCurrentIndex(axisY->getRealJoyIndex());
            ui->xAxisTwoComboBox->setEnabled(true);
            ui->yAxisTwoComboBox->setEnabled(true);
            ui->enableTwoCheckBox->setEnabled(true);
            ui->enableTwoCheckBox->setChecked(true);
        }
    }
    else
    {
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->xAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->enableTwoCheckBox->setChecked(false);
    }
}
