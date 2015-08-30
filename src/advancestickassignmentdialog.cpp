/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <typeinfo>

#include <QString>
#include <QHashIterator>
#include <QMessageBox>

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
    joystick->getActiveSetJoystick()->setIgnoreEventState(true);
    joystick->getActiveSetJoystick()->release();
    joystick->resetButtonDownCount();

    QString tempHeaderLabel = ui->joystickNumberLabel->text();
    tempHeaderLabel = tempHeaderLabel.arg(joystick->getSDLName()).arg(joystick->getRealJoyNumber());
    ui->joystickNumberLabel->setText(tempHeaderLabel);
    ui->joystickNumberLabel2->setText(tempHeaderLabel);

    tempHeaderLabel = ui->hatNumberLabel->text();
    tempHeaderLabel = tempHeaderLabel.arg(joystick->getNumberHats());
    ui->hatNumberLabel->setText(tempHeaderLabel);

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

    populateDPadComboBoxes();
    refreshVDPadConfiguration();

#ifndef USE_SDL_2
    ui->versionTwoMessageLabel->setVisible(false);
#endif

    connect(ui->enableOneCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeStateStickOneWidgets(bool)));
    connect(ui->enableTwoCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeStateStickTwoWidgets(bool)));

    connect(ui->vdpadEnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(changeStateVDPadWidgets(bool)));

    connect(ui->xAxisOneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickOne()));
    connect(ui->yAxisOneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickOne()));

    connect(ui->xAxisTwoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickTwo()));
    connect(ui->yAxisTwoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForAxisAssignmentStickTwo()));

    connect(ui->quickAssignStick1PushButton, SIGNAL(clicked()), this, SLOT(openQuickAssignDialogStick1()));
    connect(ui->quickAssignStick2PushButton, SIGNAL(clicked()), this, SLOT(openQuickAssignDialogStick2()));

    enableVDPadComboBoxes();

    connect(this, SIGNAL(stickConfigurationChanged()), this, SLOT(disableVDPadComboBoxes()));
    connect(this, SIGNAL(stickConfigurationChanged()), this, SLOT(populateDPadComboBoxes()));
    connect(this, SIGNAL(stickConfigurationChanged()), this, SLOT(refreshVDPadConfiguration()));
    connect(this, SIGNAL(stickConfigurationChanged()), this, SLOT(enableVDPadComboBoxes()));

    connect(ui->vdpadUpPushButton, SIGNAL(clicked()), this, SLOT(openAssignVDPadUp()));
    connect(ui->vdpadDownPushButton, SIGNAL(clicked()), this, SLOT(openAssignVDPadDown()));
    connect(ui->vdpadLeftPushButton, SIGNAL(clicked()), this, SLOT(openAssignVDPadLeft()));
    connect(ui->vdpadRightPushButton, SIGNAL(clicked()), this, SLOT(openAssignVDPadRight()));

    connect(this, SIGNAL(finished(int)), this, SLOT(reenableButtonEvents()));
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
                        controlstick->replaceAxes(axis1, axis2);
                    }
                    else
                    {
                        JoyControlStick *controlstick = new JoyControlStick(axis1, axis2, 0, i, currentset);
                        currentset->addControlStick(0, controlstick);
                    }
                }
            }

            refreshStickConfiguration();
            emit stickConfigurationChanged();
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
            emit stickConfigurationChanged();
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

void AdvanceStickAssignmentDialog::changeStateVDPadWidgets(bool enabled)
{
    if (enabled)
    {
        ui->vdpadUpComboBox->setEnabled(true);
        ui->vdpadDownComboBox->setEnabled(true);
        ui->vdpadLeftComboBox->setEnabled(true);
        ui->vdpadRightComboBox->setEnabled(true);

        ui->vdpadUpPushButton->setEnabled(true);
        ui->vdpadDownPushButton->setEnabled(true);
        ui->vdpadLeftPushButton->setEnabled(true);
        ui->vdpadRightPushButton->setEnabled(true);

        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            if (!currentset->getVDPad(0))
            {
                VDPad *vdpad = new VDPad(0, i, currentset, currentset);
                currentset->addVDPad(0, vdpad);
            }
        }
    }
    else
    {
        ui->vdpadUpComboBox->setEnabled(false);
        ui->vdpadDownComboBox->setEnabled(false);
        ui->vdpadLeftComboBox->setEnabled(false);
        ui->vdpadRightComboBox->setEnabled(false);

        ui->vdpadUpPushButton->setEnabled(false);
        ui->vdpadDownPushButton->setEnabled(false);
        ui->vdpadLeftPushButton->setEnabled(false);
        ui->vdpadRightPushButton->setEnabled(false);

        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            if (currentset->getVDPad(0))
            {
                currentset->removeVDPad(0);
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
        ui->quickAssignStick1PushButton->setEnabled(true);
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
        ui->quickAssignStick1PushButton->setEnabled(false);

        JoyControlStick *controlstick = joystick->getActiveSetJoystick()->getJoyStick(0);
        JoyControlStick *controlstick2 = joystick->getActiveSetJoystick()->getJoyStick(1);

        if (controlstick2)
        {
            joystick->removeControlStick(1);
        }

        if (controlstick)
        {
            joystick->removeControlStick(0);
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateStickTwoWidgets(bool enabled)
{
    if (enabled)
    {
        ui->xAxisTwoComboBox->setEnabled(true);
        ui->yAxisTwoComboBox->setEnabled(true);
        ui->quickAssignStick2PushButton->setEnabled(true);
    }
    else
    {
        ui->xAxisTwoComboBox->setEnabled(false);
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setCurrentIndex(0);
        ui->quickAssignStick2PushButton->setEnabled(false);

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
            ui->quickAssignStick1PushButton->setEnabled(true);
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
        ui->quickAssignStick1PushButton->setEnabled(false);
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
            ui->quickAssignStick2PushButton->setEnabled(true);
        }
    }
    else
    {
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->xAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->enableTwoCheckBox->setChecked(false);
        ui->quickAssignStick2PushButton->setEnabled(false);
    }
}

void AdvanceStickAssignmentDialog::refreshVDPadConfiguration()
{
    VDPad *vdpad = joystick->getActiveSetJoystick()->getVDPad(0);
    if (vdpad)
    {
        ui->vdpadEnableCheckBox->setChecked(true);
        ui->vdpadUpComboBox->setEnabled(true);
        ui->vdpadDownComboBox->setEnabled(true);
        ui->vdpadLeftComboBox->setEnabled(true);
        ui->vdpadRightComboBox->setEnabled(true);

        ui->vdpadUpPushButton->setEnabled(true);
        ui->vdpadDownPushButton->setEnabled(true);
        ui->vdpadLeftPushButton->setEnabled(true);
        ui->vdpadRightPushButton->setEnabled(true);

        JoyButton *upButton = vdpad->getVButton(JoyDPadButton::DpadUp);
        if (upButton)
        {
            int buttonindex = 0;
            if (typeid(*upButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(upButton);
                JoyAxis *axis = axisbutton->getAxis();
                QList<QVariant> templist;
                templist.append(QVariant(axis->getRealJoyIndex()));
                templist.append(QVariant(axisbutton->getJoyNumber()));
                buttonindex = ui->vdpadUpComboBox->findData(templist);
            }
            else
            {
                QList<QVariant> templist;
                templist.append(QVariant(0));
                templist.append(QVariant(upButton->getRealJoyNumber()));
                buttonindex = ui->vdpadUpComboBox->findData(templist);
            }

            if (buttonindex == -1)
            {
                vdpad->removeVButton(upButton);
            }
            else
            {
                ui->vdpadUpComboBox->setCurrentIndex(buttonindex);
            }
        }

        JoyButton *downButton = vdpad->getVButton(JoyDPadButton::DpadDown);
        if (downButton)
        {
            int buttonindex = 0;
            if (typeid(*downButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(downButton);
                JoyAxis *axis = axisbutton->getAxis();
                QList<QVariant> templist;
                templist.append(QVariant(axis->getRealJoyIndex()));
                templist.append(QVariant(axisbutton->getJoyNumber()));
                buttonindex = ui->vdpadDownComboBox->findData(templist);
            }
            else
            {
                QList<QVariant> templist;
                templist.append(QVariant(0));
                templist.append(QVariant(downButton->getRealJoyNumber()));
                buttonindex = ui->vdpadDownComboBox->findData(templist);
            }

            if (buttonindex == -1)
            {
                vdpad->removeVButton(downButton);
            }
            else
            {
                ui->vdpadDownComboBox->setCurrentIndex(buttonindex);
            }
        }

        JoyButton *leftButton = vdpad->getVButton(JoyDPadButton::DpadLeft);
        if (leftButton)
        {
            int buttonindex = 0;
            if (typeid(*leftButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(leftButton);
                JoyAxis *axis = axisbutton->getAxis();
                QList<QVariant> templist;
                templist.append(QVariant(axis->getRealJoyIndex()));
                templist.append(QVariant(axisbutton->getJoyNumber()));
                buttonindex = ui->vdpadLeftComboBox->findData(templist);
            }
            else
            {
                QList<QVariant> templist;
                templist.append(QVariant(0));
                templist.append(QVariant(leftButton->getRealJoyNumber()));
                buttonindex = ui->vdpadLeftComboBox->findData(templist);
            }

            if (buttonindex == -1)
            {
                vdpad->removeVButton(leftButton);
            }
            else
            {
                ui->vdpadLeftComboBox->setCurrentIndex(buttonindex);
            }
        }

        JoyButton *rightButton = vdpad->getVButton(JoyDPadButton::DpadRight);
        if (rightButton)
        {
            int buttonindex = 0;
            if (typeid(*rightButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(rightButton);
                JoyAxis *axis = axisbutton->getAxis();
                QList<QVariant> templist;
                templist.append(QVariant(axis->getRealJoyIndex()));
                templist.append(QVariant(axisbutton->getJoyNumber()));
                buttonindex = ui->vdpadRightComboBox->findData(templist);
            }
            else
            {
                QList<QVariant> templist;
                templist.append(QVariant(0));
                templist.append(QVariant(rightButton->getRealJoyNumber()));
                buttonindex = ui->vdpadRightComboBox->findData(templist);
            }

            if (buttonindex == -1)
            {
                vdpad->removeVButton(rightButton);
            }
            else
            {
                ui->vdpadRightComboBox->setCurrentIndex(buttonindex);
            }
        }
    }
    else
    {
        ui->vdpadEnableCheckBox->setChecked(false);

        ui->vdpadUpComboBox->setCurrentIndex(0);
        ui->vdpadUpComboBox->setEnabled(false);

        ui->vdpadDownComboBox->setCurrentIndex(0);
        ui->vdpadDownComboBox->setEnabled(false);

        ui->vdpadLeftComboBox->setCurrentIndex(0);
        ui->vdpadLeftComboBox->setEnabled(false);

        ui->vdpadRightComboBox->setCurrentIndex(0);
        ui->vdpadRightComboBox->setEnabled(false);

        ui->vdpadUpPushButton->setEnabled(false);
        ui->vdpadDownPushButton->setEnabled(false);
        ui->vdpadLeftPushButton->setEnabled(false);
        ui->vdpadRightPushButton->setEnabled(false);
    }
}

void AdvanceStickAssignmentDialog::populateDPadComboBoxes()
{
    ui->vdpadUpComboBox->clear();
    ui->vdpadDownComboBox->clear();
    ui->vdpadLeftComboBox->clear();
    ui->vdpadRightComboBox->clear();

    ui->vdpadUpComboBox->addItem("", QVariant(0));
    ui->vdpadDownComboBox->addItem("", QVariant(0));
    ui->vdpadLeftComboBox->addItem("", QVariant(0));
    ui->vdpadRightComboBox->addItem("", QVariant(0));

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (!axis->isPartControlStick())
        {
            QList<QVariant> templist;
            templist.append(QVariant(i+1));
            templist.append(QVariant(0));

            ui->vdpadUpComboBox->addItem(tr("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadDownComboBox->addItem(tr("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadLeftComboBox->addItem(tr("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadRightComboBox->addItem(tr("Axis %1 -").arg(QString::number(i+1)), templist);

            templist.clear();
            templist.append(QVariant(i+1));
            templist.append(QVariant(1));
            ui->vdpadUpComboBox->addItem(tr("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadDownComboBox->addItem(tr("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadLeftComboBox->addItem(tr("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadRightComboBox->addItem(tr("Axis %1 +").arg(QString::number(i+1)), templist);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(i+1));

        ui->vdpadUpComboBox->addItem(tr("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadDownComboBox->addItem(tr("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadLeftComboBox->addItem(tr("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadRightComboBox->addItem(tr("Button %1").arg(QString::number(i+1)), templist);
    }
}

void AdvanceStickAssignmentDialog::changeVDPadUpButton(int index)
{
    if (index > 0)
    {
        if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadLeftComboBox->currentIndex() == index)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadRightComboBox->currentIndex() == index)
        {
            ui->vdpadRightComboBox->setCurrentIndex(0);
        }

        QVariant temp = ui->vdpadUpComboBox->itemData(index);
        QList<QVariant> templist = temp.toList();
        if (templist.size() == 2)
        {
            int axis = templist.at(0).toInt();
            int button = templist.at(1).toInt();
            if (axis > 0 && button >= 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = 0;
                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    }
                    else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadUp, currentbutton);
                }
            }
            else if (button > 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button-1);
                    if (currentbutton)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadUp, currentbutton);
                    }
                }
            }
        }
    }
    else
    {
        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            VDPad *vdpad = currentset->getVDPad(0);
            if (vdpad && vdpad->getVButton(JoyDPadButton::DpadUp))
            {
                vdpad->removeVButton(JoyDPadButton::DpadUp);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadDownButton(int index)
{
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadLeftComboBox->currentIndex() == index)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadRightComboBox->currentIndex() == index)
        {
            ui->vdpadRightComboBox->setCurrentIndex(0);
        }

        QVariant temp = ui->vdpadDownComboBox->itemData(index);
        QList<QVariant> templist = temp.toList();
        if (templist.size() == 2)
        {
            int axis = templist.at(0).toInt();
            int button = templist.at(1).toInt();
            if (axis > 0 && button >= 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = 0;
                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    }
                    else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadDown, currentbutton);
                }
            }
            else if (button > 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button-1);
                    if (currentbutton)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadDown, currentbutton);
                    }
                }
            }
        }
    }
    else
    {
        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            VDPad *vdpad = currentset->getVDPad(0);
            if (vdpad && vdpad->getVButton(JoyDPadButton::DpadDown))
            {
                vdpad->removeVButton(JoyDPadButton::DpadDown);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadLeftButton(int index)
{
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadRightComboBox->currentIndex() == index)
        {
            ui->vdpadRightComboBox->setCurrentIndex(0);
        }

        QVariant temp = ui->vdpadLeftComboBox->itemData(index);
        QList<QVariant> templist = temp.toList();
        if (templist.size() == 2)
        {
            int axis = templist.at(0).toInt();
            int button = templist.at(1).toInt();
            if (axis > 0 && button >= 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = 0;
                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    }
                    else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadLeft, currentbutton);
                }
            }
            else if (button > 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button-1);
                    if (currentbutton)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadLeft, currentbutton);
                    }
                }
            }
        }
    }
    else
    {
        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            VDPad *vdpad = currentset->getVDPad(0);
            if (vdpad && vdpad->getVButton(JoyDPadButton::DpadLeft))
            {
                vdpad->removeVButton(JoyDPadButton::DpadLeft);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadRightButton(int index)
{
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        }
        else if (ui->vdpadLeftComboBox->currentIndex() == index)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(0);
        }

        QVariant temp = ui->vdpadRightComboBox->itemData(index);
        QList<QVariant> templist = temp.toList();
        if (templist.size() == 2)
        {
            int axis = templist.at(0).toInt();
            int button = templist.at(1).toInt();
            if (axis > 0 && button >= 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = 0;
                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    }
                    else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadRight, currentbutton);
                }
            }
            else if (button > 0)
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button-1);
                    if (currentbutton)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadRight, currentbutton);
                    }
                }
            }
        }
    }
    else
    {
        for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
        {
            SetJoystick *currentset = joystick->getSetJoystick(i);
            VDPad *vdpad = currentset->getVDPad(0);
            if (vdpad && vdpad->getVButton(JoyDPadButton::DpadRight))
            {
                vdpad->removeVButton(JoyDPadButton::DpadRight);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::enableVDPadComboBoxes()
{
    connect(ui->vdpadUpComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadUpButton(int)));
    connect(ui->vdpadDownComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadDownButton(int)));
    connect(ui->vdpadLeftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadLeftButton(int)));
    connect(ui->vdpadRightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadRightButton(int)));
}

void AdvanceStickAssignmentDialog::disableVDPadComboBoxes()
{
    disconnect(ui->vdpadUpComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadUpButton(int)));
    disconnect(ui->vdpadDownComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadDownButton(int)));
    disconnect(ui->vdpadLeftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadLeftButton(int)));
    disconnect(ui->vdpadRightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeVDPadRightButton(int)));
}

void AdvanceStickAssignmentDialog::openQuickAssignDialogStick1()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Move stick 1 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            connect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            connect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick1Axis1()));
        }
    }

    msgBox.exec();

    msgBox.setText(tr("Move stick 1 along the Y axis"));
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            disconnect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            disconnect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick1Axis1()));

            connect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            connect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick1Axis2()));
        }
    }

    msgBox.exec();
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            disconnect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            disconnect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick1Axis2()));
        }
    }
}

void AdvanceStickAssignmentDialog::openQuickAssignDialogStick2()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Move stick 2 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            connect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            connect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick2Axis1()));
        }
    }

    msgBox.exec();


    msgBox.setText(tr("Move stick 2 along the Y axis"));
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            disconnect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            disconnect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick2Axis1()));

            connect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            connect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick2Axis2()));
        }
    }

    msgBox.exec();
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis)
        {
            disconnect(axis, SIGNAL(active(int)), &msgBox, SLOT(close()));
            disconnect(axis, SIGNAL(active(int)), this, SLOT(quickAssignStick2Axis2()));
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignStick1Axis1()
{
    JoyAxis *axis = static_cast<JoyAxis*>(sender());
    ui->xAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex());
}

void AdvanceStickAssignmentDialog::quickAssignStick1Axis2()
{
    JoyAxis *axis = static_cast<JoyAxis*>(sender());
    ui->yAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex());
}

void AdvanceStickAssignmentDialog::quickAssignStick2Axis1()
{
    JoyAxis *axis = static_cast<JoyAxis*>(sender());
    ui->xAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex());
}

void AdvanceStickAssignmentDialog::quickAssignStick2Axis2()
{
    JoyAxis *axis = static_cast<JoyAxis*>(sender());
    ui->yAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex());
}

void AdvanceStickAssignmentDialog::reenableButtonEvents()
{
    joystick->getActiveSetJoystick()->setIgnoreEventState(false);
    joystick->getActiveSetJoystick()->release();
}

void AdvanceStickAssignmentDialog::openAssignVDPadUp()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));

            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            connect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));
        }
    }

    msgBox.exec();

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));

            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            disconnect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadUp()));
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadDown()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));

            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            connect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));
        }
    }

    msgBox.exec();

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));

            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            disconnect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadDown()));
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadLeft()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));

            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            connect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));
        }
    }

    msgBox.exec();

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));

            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            disconnect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadLeft()));
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadRight()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));

            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            connect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            connect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));
        }
    }

    msgBox.exec();

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getNAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));

            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(axis->getPAxisButton(), SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));
        }
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            disconnect(button, SIGNAL(clicked(int)), &msgBox, SLOT(close()));
            disconnect(button, SIGNAL(clicked(int)), this, SLOT(quickAssignVDPadRight()));
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadUp()
{
    if (qobject_cast<JoyAxisButton*>(sender()) != 0)
    {
        JoyAxisButton *axisButton = static_cast<JoyAxisButton*>(sender());

        QList<QVariant> templist;
        templist.append(QVariant(axisButton->getAxis()->getRealJoyIndex()));
        if (axisButton->getAxis()->getNAxisButton() == axisButton)
        {
            templist.append(QVariant(0));
        }
        else
        {
            templist.append(QVariant(1));
        }

        int index = ui->vdpadUpComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadUpComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(button->getJoyNumber()+1));

        int index = ui->vdpadUpComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadUpComboBox->setCurrentIndex(index);
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadDown()
{
    if (qobject_cast<JoyAxisButton*>(sender()) != 0)
    {
        JoyAxisButton *axisButton = static_cast<JoyAxisButton*>(sender());

        QList<QVariant> templist;
        templist.append(QVariant(axisButton->getAxis()->getRealJoyIndex()));
        if (axisButton->getAxis()->getNAxisButton() == axisButton)
        {
            templist.append(QVariant(0));
        }
        else
        {
            templist.append(QVariant(1));
        }

        int index = ui->vdpadDownComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadDownComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(button->getJoyNumber()+1));

        int index = ui->vdpadDownComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadDownComboBox->setCurrentIndex(index);
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadLeft()
{
    if (qobject_cast<JoyAxisButton*>(sender()) != 0)
    {
        JoyAxisButton *axisButton = static_cast<JoyAxisButton*>(sender());

        QList<QVariant> templist;
        templist.append(QVariant(axisButton->getAxis()->getRealJoyIndex()));
        if (axisButton->getAxis()->getNAxisButton() == axisButton)
        {
            templist.append(QVariant(0));
        }
        else
        {
            templist.append(QVariant(1));
        }

        int index = ui->vdpadLeftComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(button->getJoyNumber()+1));

        int index = ui->vdpadLeftComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(index);
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadRight()
{
    if (qobject_cast<JoyAxisButton*>(sender()) != 0)
    {
        JoyAxisButton *axisButton = static_cast<JoyAxisButton*>(sender());

        QList<QVariant> templist;
        templist.append(QVariant(axisButton->getAxis()->getRealJoyIndex()));
        if (axisButton->getAxis()->getNAxisButton() == axisButton)
        {
            templist.append(QVariant(0));
        }
        else
        {
            templist.append(QVariant(1));
        }

        int index = ui->vdpadRightComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadRightComboBox->setCurrentIndex(index);
        }
    }
    else
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(button->getJoyNumber()+1));

        int index = ui->vdpadRightComboBox->findData(templist);
        if (index > 0)
        {
            ui->vdpadRightComboBox->setCurrentIndex(index);
        }
    }
}
