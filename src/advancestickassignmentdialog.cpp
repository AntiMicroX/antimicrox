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

#include "advancestickassignmentdialog.h"
#include "ui_advancestickassignmentdialog.h"

#include "messagehandler.h"
#include "joycontrolstick.h"
#include "joystick.h"
#include "vdpad.h"

#include <typeinfo>

#include <QString>
#include <QHashIterator>
#include <QMessageBox>
#include <QList>
#include <QVariant>
#include <QSignalMapper>
#include <QDebug>


AdvanceStickAssignmentDialog::AdvanceStickAssignmentDialog(Joystick *joystick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AdvanceStickAssignmentDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
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
        ui->xAxisOneComboBox->addItem(trUtf8("Axis %1").arg(i+1), QVariant(i));
        ui->yAxisOneComboBox->addItem(trUtf8("Axis %1").arg(i+1), QVariant(i));

        ui->xAxisTwoComboBox->addItem(trUtf8("Axis %1").arg(i+1), QVariant(i));
        ui->yAxisTwoComboBox->addItem(trUtf8("Axis %1").arg(i+1), QVariant(i));
    }

    refreshStickConfiguration();

    populateDPadComboBoxes();
    refreshVDPadConfiguration();

    ui->versionTwoMessageLabel->setVisible(false);



    connect(ui->enableOneCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateStickOneWidgets);
    connect(ui->enableTwoCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateStickTwoWidgets);

    connect(ui->vdpadEnableCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateVDPadWidgets);

    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, static_cast<void (QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped), this, &AdvanceStickAssignmentDialog::checkForAxisAssignmentStickOne);
    connect(signalMapper, static_cast<void (QSignalMapper::*)(QWidget *)>(&QSignalMapper::mapped), this, &AdvanceStickAssignmentDialog::checkForAxisAssignmentStickTwo);

    signalMapper->setMapping(ui->xAxisOneComboBox, ui->xAxisOneComboBox);
    signalMapper->setMapping(ui->yAxisOneComboBox, ui->yAxisOneComboBox);

    connect(ui->xAxisOneComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [signalMapper]() { signalMapper->map(); });
    connect(ui->yAxisOneComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [signalMapper]() { signalMapper->map(); });

    connect(ui->quickAssignStick1PushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openQuickAssignDialogStick1);
    connect(ui->quickAssignStick2PushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openQuickAssignDialogStick2);

    enableVDPadComboBoxes();

    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this, &AdvanceStickAssignmentDialog::disableVDPadComboBoxes);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this, &AdvanceStickAssignmentDialog::populateDPadComboBoxes);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this, &AdvanceStickAssignmentDialog::refreshVDPadConfiguration);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this, &AdvanceStickAssignmentDialog::enableVDPadComboBoxes);

    connect(ui->vdpadUpPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadUp);
    connect(ui->vdpadDownPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadDown);
    connect(ui->vdpadLeftPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadLeft);
    connect(ui->vdpadRightPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadRight);

    connect(this, &AdvanceStickAssignmentDialog::finished, this, &AdvanceStickAssignmentDialog::reenableButtonEvents);
}

AdvanceStickAssignmentDialog::~AdvanceStickAssignmentDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickOne(QWidget* comboBox)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((ui->xAxisOneComboBox->currentIndex() > 0) && (ui->yAxisOneComboBox->currentIndex() > 0))
    {
        if (ui->xAxisOneComboBox->currentIndex() != ui->yAxisOneComboBox->currentIndex())
        {
            for (int i = 0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                JoyAxis *axis1 = currentset->getJoyAxis(ui->xAxisOneComboBox->currentIndex()-1);
                JoyAxis *axis2 = currentset->getJoyAxis(ui->yAxisOneComboBox->currentIndex()-1);

                if (axis1 && axis2)
                {
                    JoyControlStick *controlstick = currentset->getJoyStick(0);
                    if (controlstick != nullptr)
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
            if (comboBox == ui->xAxisOneComboBox)
            {
                ui->yAxisOneComboBox->setCurrentIndex(0);
            }
            else if (comboBox == ui->yAxisOneComboBox)
            {
                ui->xAxisOneComboBox->setCurrentIndex(0);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickTwo(QWidget* comboBox)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((ui->xAxisTwoComboBox->currentIndex() > 0) && (ui->yAxisTwoComboBox->currentIndex() > 0))
    {
        if (ui->xAxisTwoComboBox->currentIndex() != ui->yAxisTwoComboBox->currentIndex())
        {
            for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
            {
                SetJoystick *currentset = joystick->getSetJoystick(i);
                JoyAxis *axis1 = currentset->getJoyAxis(ui->xAxisTwoComboBox->currentIndex()-1);
                JoyAxis *axis2 = currentset->getJoyAxis(ui->yAxisTwoComboBox->currentIndex()-1);
                if ((axis1 != nullptr) && (axis2 != nullptr))
                {
                    JoyControlStick *controlstick = currentset->getJoyStick(1);
                    if (controlstick != nullptr)
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
            if (comboBox == ui->xAxisTwoComboBox)
            {
                ui->yAxisTwoComboBox->setCurrentIndex(0);
            }
            else if (comboBox == ui->yAxisTwoComboBox)
            {
                ui->xAxisTwoComboBox->setCurrentIndex(0);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateVDPadWidgets(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

        if (controlstick2 != nullptr)
        {
            joystick->removeControlStick(1);
        }

        if (controlstick != nullptr)
        {
            joystick->removeControlStick(0);
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateStickTwoWidgets(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
        if (controlstick != nullptr)
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStick *stick1 = joystick->getActiveSetJoystick()->getJoyStick(0);
    JoyControlStick *stick2 = joystick->getActiveSetJoystick()->getJoyStick(1);
    if (stick1)
    {
        JoyAxis *axisX = stick1->getAxisX();
        JoyAxis *axisY = stick1->getAxisY();
        if ((axisX != nullptr) && (axisY != nullptr))
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
        if ((axisX != nullptr) && (axisY != nullptr))
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    VDPad *vdpad = joystick->getActiveSetJoystick()->getVDPad(0);
    if (vdpad != nullptr)
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
        if (upButton != nullptr)
        {
            int buttonindex = 0;
            if (typeid(*upButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(upButton); // static_cast
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
        if (downButton != nullptr)
        {
            int buttonindex = 0;
            if (typeid(*downButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(downButton); // static_cast
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
        if (leftButton != nullptr)
        {
            int buttonindex = 0;
            if (typeid(*leftButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(leftButton); // static_cast
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
        if (rightButton != nullptr)
        {
            int buttonindex = 0;
            if (typeid(*rightButton) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(rightButton); // static_cast
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

            ui->vdpadUpComboBox->addItem(trUtf8("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadDownComboBox->addItem(trUtf8("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadLeftComboBox->addItem(trUtf8("Axis %1 -").arg(QString::number(i+1)), templist);
            ui->vdpadRightComboBox->addItem(trUtf8("Axis %1 -").arg(QString::number(i+1)), templist);

            templist.clear();
            templist.append(QVariant(i+1));
            templist.append(QVariant(1));
            ui->vdpadUpComboBox->addItem(trUtf8("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadDownComboBox->addItem(trUtf8("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadLeftComboBox->addItem(trUtf8("Axis %1 +").arg(QString::number(i+1)), templist);
            ui->vdpadRightComboBox->addItem(trUtf8("Axis %1 +").arg(QString::number(i+1)), templist);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(i+1));

        ui->vdpadUpComboBox->addItem(trUtf8("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadDownComboBox->addItem(trUtf8("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadLeftComboBox->addItem(trUtf8("Button %1").arg(QString::number(i+1)), templist);
        ui->vdpadRightComboBox->addItem(trUtf8("Button %1").arg(QString::number(i+1)), templist);
    }
}

void AdvanceStickAssignmentDialog::changeVDPadUpButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            if ((axis > 0) && (button >= 0))
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = nullptr;
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
                    if (currentbutton != nullptr)
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
            if ((vdpad != nullptr) && vdpad->getVButton(JoyDPadButton::DpadUp))
            {
                vdpad->removeVButton(JoyDPadButton::DpadUp);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadDownButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            if ((axis > 0) && (button >= 0))
            {
                for (int i = 0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = nullptr;
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
                    if (currentbutton != nullptr)
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
            if ((vdpad != nullptr) && vdpad->getVButton(JoyDPadButton::DpadDown))
            {
                vdpad->removeVButton(JoyDPadButton::DpadDown);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadLeftButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            if ((axis > 0) && (button >= 0))
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = nullptr;
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
                    if (currentbutton != nullptr)
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
            if ((vdpad != nullptr) && vdpad->getVButton(JoyDPadButton::DpadLeft))
            {
                vdpad->removeVButton(JoyDPadButton::DpadLeft);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::changeVDPadRightButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            if ((axis > 0) && (button >= 0))
            {
                for (int i=0; i < joystick->NUMBER_JOYSETS; i++)
                {
                    SetJoystick *currentset = joystick->getSetJoystick(i);
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis-1);
                    JoyButton *currentbutton = nullptr;
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
                    if (currentbutton != nullptr)
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
            if ((vdpad != nullptr) && vdpad->getVButton(JoyDPadButton::DpadRight))
            {
                vdpad->removeVButton(JoyDPadButton::DpadRight);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::enableVDPadComboBoxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(ui->vdpadUpComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadUpButton);
    connect(ui->vdpadDownComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadDownButton);
    connect(ui->vdpadLeftComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadLeftButton);
    connect(ui->vdpadRightComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadRightButton);
}

void AdvanceStickAssignmentDialog::disableVDPadComboBoxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(ui->vdpadUpComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadUpButton);
    disconnect(ui->vdpadDownComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadDownButton);
    disconnect(ui->vdpadLeftComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadLeftButton);
    disconnect(ui->vdpadRightComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceStickAssignmentDialog::changeVDPadRightButton);
}

void AdvanceStickAssignmentDialog::openQuickAssignDialogStick1()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Move stick 1 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this, [this, axis] {

                ui->xAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex());
            });
        }
    }

    msgBox.exec();

    msgBox.setText(trUtf8("Move stick 1 along the Y axis"));
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);

            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this, [this, axis] {
                ui->yAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex());
            });
        }
    }

    msgBox.exec();
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::openQuickAssignDialogStick2()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Move stick 2 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this, [this, axis] {
                ui->xAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex());
            });
        }
    }

    msgBox.exec();


    msgBox.setText(trUtf8("Move stick 2 along the Y axis"));
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);

            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this, [this, axis] {
               ui->yAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex());
            });
        }
    }

    msgBox.exec();
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::reenableButtonEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    joystick->getActiveSetJoystick()->setIgnoreEventState(false);
    joystick->getActiveSetJoystick()->release();
}

void AdvanceStickAssignmentDialog::openAssignVDPadUp()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton* axbtn = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, axbtn] {
                quickAssignVDPadUp(axbtn);
            });

            JoyAxisButton* axbtnp = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, axbtnp] {
                quickAssignVDPadUp(axbtnp);
            });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button]() {
               quickAssignVDPadUpBtn(button);
            });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, nullptr);

            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, nullptr);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            disconnect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(button, &JoyButton::clicked, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadDown()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton* axbtn = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, axbtn] {
                quickAssignVDPadDown(axbtn);
            });

            JoyAxisButton* axbtnP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, axbtnP] {
                quickAssignVDPadDown(axbtnP);
            });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] {
               quickAssignVDPadDownJbtn(button);
            });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, nullptr);

            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, nullptr);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            disconnect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(button, &JoyButton::clicked, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadLeft()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton* joyaxisN = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, joyaxisN]() {
                quickAssignVDPadLeft(joyaxisN);
            });

            JoyAxisButton* joyaxisP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, joyaxisP]() {
                quickAssignVDPadLeft(joyaxisP);
            });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] {
                quickAssignVDPadLeftJbtn(button);
            });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, nullptr);

            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, nullptr);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            disconnect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(button, &JoyButton::clicked, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::openAssignVDPadRight()
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(trUtf8("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton* joyaxisbtnN = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, joyaxisbtnN] {
               quickAssignVDPadRight(joyaxisbtnN);
            });

            JoyAxisButton* joyaxisbtnP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, joyaxisbtnP] {
                quickAssignVDPadRight(joyaxisbtnP);
             });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] {
               quickAssignVDPadRightJbtn(button);
            });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, nullptr);

            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, nullptr);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            disconnect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            disconnect(button, &JoyButton::clicked, this, nullptr);
        }
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadUp(JoyAxisButton* joyaxisbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        QList<QVariant> templist;
        templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));
        if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
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

void AdvanceStickAssignmentDialog::quickAssignVDPadUpBtn(JoyButton* joybtn) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber()+1));

    int index = ui->vdpadUpComboBox->findData(templist);
    if (index > 0)
    {
        ui->vdpadUpComboBox->setCurrentIndex(index);
    }
}


void AdvanceStickAssignmentDialog::quickAssignVDPadDown(JoyAxisButton* axbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        QList<QVariant> templist;
        templist.append(QVariant(axbtn->getAxis()->getRealJoyIndex()));
        if (axbtn->getAxis()->getNAxisButton() == axbtn)
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

void AdvanceStickAssignmentDialog::quickAssignVDPadDownJbtn(JoyButton* joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber()+1));

    int index = ui->vdpadDownComboBox->findData(templist);
    if (index > 0)
    {
        ui->vdpadDownComboBox->setCurrentIndex(index);
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadLeft(JoyAxisButton* joyaxisbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        QList<QVariant> templist;
        templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));
        if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
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

void AdvanceStickAssignmentDialog::quickAssignVDPadLeftJbtn(JoyButton* joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber() + 1));

    int index = ui->vdpadLeftComboBox->findData(templist);
    if (index > 0)
    {
        ui->vdpadLeftComboBox->setCurrentIndex(index);
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadRight(JoyAxisButton* joyaxisbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        QList<QVariant> templist;
        templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));
        if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
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

void AdvanceStickAssignmentDialog::quickAssignVDPadRightJbtn(JoyButton* joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber()+1));

    int index = ui->vdpadRightComboBox->findData(templist);
    if (index > 0)
    {
        ui->vdpadRightComboBox->setCurrentIndex(index);
    }
}

Joystick *AdvanceStickAssignmentDialog::getJoystick() const {

    return joystick;
}
