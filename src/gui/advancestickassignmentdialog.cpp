/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "globalvariables.h"
#include "joycontrolstick.h"
#include "joystick.h"
#include "vdpad.h"

#include <typeinfo>

#include <QDebug>
#include <QHashIterator>
#include <QList>
#include <QMessageBox>
#include <QSignalMapper>
#include <QString>
#include <QVariant>

AdvanceStickAssignmentDialog::AdvanceStickAssignmentDialog(Joystick *joystick, QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AdvanceStickAssignmentDialog)
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

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        ui->xAxisOneComboBox->addItem(tr("Axis %1").arg(i + 1), QVariant(i));
        ui->yAxisOneComboBox->addItem(tr("Axis %1").arg(i + 1), QVariant(i));

        ui->xAxisTwoComboBox->addItem(tr("Axis %1").arg(i + 1), QVariant(i));
        ui->yAxisTwoComboBox->addItem(tr("Axis %1").arg(i + 1), QVariant(i));
    }

    JoyControlStick *stick1 = joystick->getActiveSetJoystick()->getJoyStick(0);
    JoyControlStick *stick2 = joystick->getActiveSetJoystick()->getJoyStick(1);
    refreshStickConfiguration(stick1, stick2);
    populateDPadComboBoxes();
    refreshVDPadsConfiguration();

    ui->versionTwoMessageLabel->setVisible(false);

    connect(ui->enableOneCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateStickOneWidgets);
    connect(ui->enableTwoCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateStickTwoWidgets);
    connect(ui->vdpadEnableCheckBox, &QCheckBox::clicked, this, &AdvanceStickAssignmentDialog::changeStateVDPadWidgets);

    connect(ui->xAxisOneComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this]() { this->checkForAxisAssignmentStickOne(ui->xAxisOneComboBox); });

    connect(ui->yAxisOneComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this]() { this->checkForAxisAssignmentStickOne(ui->yAxisOneComboBox); });

    connect(ui->xAxisTwoComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this]() { this->checkForAxisAssignmentStickTwo(ui->xAxisTwoComboBox); });

    connect(ui->yAxisTwoComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this]() { this->checkForAxisAssignmentStickTwo(ui->yAxisTwoComboBox); });

    connect(ui->quickAssignStick1PushButton, &QPushButton::clicked, this,
            &AdvanceStickAssignmentDialog::openQuickAssignDialogStick1);
    connect(ui->quickAssignStick2PushButton, &QPushButton::clicked, this,
            &AdvanceStickAssignmentDialog::openQuickAssignDialogStick2);

    enableVDPadComboBoxes();

    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this,
            &AdvanceStickAssignmentDialog::disableVDPadComboBoxes);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this,
            &AdvanceStickAssignmentDialog::populateDPadComboBoxes);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this,
            &AdvanceStickAssignmentDialog::refreshVDPadsConfiguration);
    connect(this, &AdvanceStickAssignmentDialog::stickConfigurationChanged, this,
            &AdvanceStickAssignmentDialog::enableVDPadComboBoxes);

    connect(ui->vdpadUpPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadUp);
    connect(ui->vdpadDownPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadDown);
    connect(ui->vdpadLeftPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadLeft);
    connect(ui->vdpadRightPushButton, &QPushButton::clicked, this, &AdvanceStickAssignmentDialog::openAssignVDPadRight);

    connect(this, &AdvanceStickAssignmentDialog::finished, this, &AdvanceStickAssignmentDialog::reenableButtonEvents);
}

// for tests
AdvanceStickAssignmentDialog::AdvanceStickAssignmentDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AdvanceStickAssignmentDialog)
{
}

AdvanceStickAssignmentDialog::~AdvanceStickAssignmentDialog() { delete ui; }

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickOne(QWidget *comboBox)
{
    checkForAxisAssignmentSticks(comboBox, ui->xAxisTwoComboBox, ui->yAxisTwoComboBox, 0);
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentStickTwo(QWidget *comboBox)
{
    checkForAxisAssignmentSticks(comboBox, ui->xAxisTwoComboBox, ui->yAxisTwoComboBox, 1);
}

void AdvanceStickAssignmentDialog::checkForAxisAssignmentSticks(QWidget *comboBox, QComboBox *xAxisComboBox,
                                                                QComboBox *yAxisComboBox, int controlStickNumber)
{
    if ((xAxisComboBox->currentIndex() > 0) && (yAxisComboBox->currentIndex() > 0))
    {
        if (xAxisComboBox->currentIndex() != yAxisComboBox->currentIndex())
        {
            int originset = 0;

            for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
            {
                SetJoystick *currentset = set.value();
                JoyAxis *axis1 = currentset->getJoyAxis(xAxisComboBox->currentIndex() - 1);
                JoyAxis *axis2 = currentset->getJoyAxis(yAxisComboBox->currentIndex() - 1);

                if ((axis1 != nullptr) && (axis2 != nullptr) && (currentset->getJoyStick(controlStickNumber) != nullptr))
                {
                    currentset->getJoyStick(controlStickNumber)->replaceAxes(axis1, axis2);
                } else if ((axis1 != nullptr) && (axis2 != nullptr) &&
                           (currentset->getJoyStick(controlStickNumber) == nullptr))
                {
                    JoyControlStick *controlstick =
                        new JoyControlStick(axis1, axis2, controlStickNumber, originset, currentset);
                    currentset->addControlStick(controlStickNumber, controlstick);
                }

                originset++;
            }

            JoyControlStick *stick1 = joystick->getActiveSetJoystick()->getJoyStick(0);
            JoyControlStick *stick2 = joystick->getActiveSetJoystick()->getJoyStick(1);
            refreshStickConfiguration(stick1, stick2);
            emit stickConfigurationChanged();
        } else if (comboBox == xAxisComboBox)
        {
            yAxisComboBox->setCurrentIndex(0);
        } else if (comboBox == yAxisComboBox)
        {
            xAxisComboBox->setCurrentIndex(0);
        }
    }
}

void AdvanceStickAssignmentDialog::changeStateVDPadWidgets(bool enabledVDPads)
{
    ui->vdpadUpComboBox->setEnabled(enabledVDPads);
    ui->vdpadDownComboBox->setEnabled(enabledVDPads);
    ui->vdpadLeftComboBox->setEnabled(enabledVDPads);
    ui->vdpadRightComboBox->setEnabled(enabledVDPads);

    ui->vdpadUpPushButton->setEnabled(enabledVDPads);
    ui->vdpadDownPushButton->setEnabled(enabledVDPads);
    ui->vdpadLeftPushButton->setEnabled(enabledVDPads);
    ui->vdpadRightPushButton->setEnabled(enabledVDPads);

    int originset = 0;

    for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
    {
        SetJoystick *currentset = set.value();

        if (!currentset->getVDPad(0) && enabledVDPads)
        {
            currentset->addVDPad(0, new VDPad(0, originset, currentset, currentset));
        } else
        {
            currentset->removeVDPad(0);
        }

        originset++;
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
    } else
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

        if (joystick->getActiveSetJoystick()->getJoyStick(1) != nullptr)
            joystick->removeControlStick(1);

        if (joystick->getActiveSetJoystick()->getJoyStick(0) != nullptr)
            joystick->removeControlStick(0);
    }
}

void AdvanceStickAssignmentDialog::changeStateStickTwoWidgets(bool enabled)
{
    if (enabled)
    {
        ui->xAxisTwoComboBox->setEnabled(true);
        ui->yAxisTwoComboBox->setEnabled(true);
        ui->quickAssignStick2PushButton->setEnabled(true);
    } else
    {
        ui->xAxisTwoComboBox->setEnabled(false);
        ui->xAxisTwoComboBox->setCurrentIndex(0);
        ui->yAxisTwoComboBox->setEnabled(false);
        ui->yAxisTwoComboBox->setCurrentIndex(0);
        ui->quickAssignStick2PushButton->setEnabled(false);

        if (joystick->getActiveSetJoystick()->getJoyStick(1) != nullptr)
        {
            for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
            {
                set.value()->removeControlStick(1);
            }
        }
    }
}

void AdvanceStickAssignmentDialog::refreshStickConfiguration(JoyControlStick *stick1, JoyControlStick *stick2)
{
    if ((stick1 != nullptr) && (stick1->getAxisX() != nullptr) && (stick1->getAxisY() != nullptr))
    {
        refreshSticksForAxes(true, stick1->getAxisX()->getRealJoyIndex(), stick1->getAxisY()->getRealJoyIndex(),
                             ui->xAxisOneComboBox, ui->yAxisOneComboBox, ui->enableOneCheckBox,
                             ui->quickAssignStick1PushButton);
    } else
    {
        refreshSticksForAxes(false, 0, 0, ui->xAxisOneComboBox, ui->yAxisOneComboBox, ui->enableOneCheckBox,
                             ui->quickAssignStick1PushButton);
    }

    if ((stick2 != nullptr) && (stick2->getAxisX() != nullptr) && (stick2->getAxisY() != nullptr))
    {
        refreshSticksForAxes(true, stick2->getAxisX()->getRealJoyIndex(), stick2->getAxisY()->getRealJoyIndex(),
                             ui->xAxisTwoComboBox, ui->yAxisTwoComboBox, ui->enableTwoCheckBox,
                             ui->quickAssignStick2PushButton);
    } else
    {
        refreshSticksForAxes(false, 0, 0, ui->xAxisTwoComboBox, ui->yAxisTwoComboBox, ui->enableTwoCheckBox,
                             ui->quickAssignStick2PushButton);
    }
}

void AdvanceStickAssignmentDialog::refreshSticksForAxes(bool axesExist, int xAxisComboBoxIndex, int yAxisComboBoxIndex,
                                                        QComboBox *xAxisComboBox, QComboBox *yAxisComboBox,
                                                        QCheckBox *enabledSticksCheckbox, QPushButton *quickAssignBtn)
{
    xAxisComboBox->setCurrentIndex(xAxisComboBoxIndex);
    yAxisComboBox->setCurrentIndex(yAxisComboBoxIndex);
    xAxisComboBox->setEnabled(axesExist);
    yAxisComboBox->setEnabled(axesExist);
    enabledSticksCheckbox->setEnabled(axesExist);
    enabledSticksCheckbox->setChecked(axesExist);
    quickAssignBtn->setEnabled(axesExist);
}

void AdvanceStickAssignmentDialog::refreshVDPadsConfiguration()
{
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
        refreshVDPadConf(upButton, ui->vdpadUpComboBox);

        JoyButton *downButton = vdpad->getVButton(JoyDPadButton::DpadDown);
        refreshVDPadConf(downButton, ui->vdpadDownComboBox);

        JoyButton *leftButton = vdpad->getVButton(JoyDPadButton::DpadLeft);
        refreshVDPadConf(leftButton, ui->vdpadLeftComboBox);

        JoyButton *rightButton = vdpad->getVButton(JoyDPadButton::DpadRight);
        refreshVDPadConf(rightButton, ui->vdpadRightComboBox);
    } else
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

void AdvanceStickAssignmentDialog::refreshVDPadConf(JoyButton *vdpadButton, QComboBox *vpadComboBox)
{
    if (vdpadButton != nullptr)
    {
        int buttonindex = 0;

        if (typeid(*vdpadButton) == typeid(JoyAxisButton))
        {
            JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton *>(vdpadButton);
            JoyAxis *axis = axisbutton->getAxis();
            QList<QVariant> templist;
            templist.append(QVariant(axis->getRealJoyIndex()));
            templist.append(QVariant(axisbutton->getJoyNumber()));
            buttonindex = vpadComboBox->findData(templist);
        } else
        {
            QList<QVariant> templist;
            templist.append(QVariant(0));
            templist.append(QVariant(vdpadButton->getRealJoyNumber()));
            buttonindex = vpadComboBox->findData(templist);
        }

        if (buttonindex == -1)
            vdpadButton->getVDPad()->removeVButton(vdpadButton);
        else
            vpadComboBox->setCurrentIndex(buttonindex);
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
            templist.append(QVariant(i + 1));
            templist.append(QVariant(0));

            ui->vdpadUpComboBox->addItem(tr("Axis %1 -").arg(QString::number(i + 1)), templist);
            ui->vdpadDownComboBox->addItem(tr("Axis %1 -").arg(QString::number(i + 1)), templist);
            ui->vdpadLeftComboBox->addItem(tr("Axis %1 -").arg(QString::number(i + 1)), templist);
            ui->vdpadRightComboBox->addItem(tr("Axis %1 -").arg(QString::number(i + 1)), templist);

            templist.clear();
            templist.append(QVariant(i + 1));
            templist.append(QVariant(1));

            ui->vdpadUpComboBox->addItem(tr("Axis %1 +").arg(QString::number(i + 1)), templist);
            ui->vdpadDownComboBox->addItem(tr("Axis %1 +").arg(QString::number(i + 1)), templist);
            ui->vdpadLeftComboBox->addItem(tr("Axis %1 +").arg(QString::number(i + 1)), templist);
            ui->vdpadRightComboBox->addItem(tr("Axis %1 +").arg(QString::number(i + 1)), templist);
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(i + 1));

        ui->vdpadUpComboBox->addItem(tr("Button %1").arg(QString::number(i + 1)), templist);
        ui->vdpadDownComboBox->addItem(tr("Button %1").arg(QString::number(i + 1)), templist);
        ui->vdpadLeftComboBox->addItem(tr("Button %1").arg(QString::number(i + 1)), templist);
        ui->vdpadRightComboBox->addItem(tr("Button %1").arg(QString::number(i + 1)), templist);
    }
}

void AdvanceStickAssignmentDialog::changeVDPadUpButton(int index)
{
    if (index > 0)
    {
        if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        } else if (ui->vdpadLeftComboBox->currentIndex() == index)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(0);
        } else if (ui->vdpadRightComboBox->currentIndex() == index)
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

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis - 1);
                    JoyButton *currentbutton = nullptr;

                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    } else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadUp, currentbutton);
                }
            } else if (button > 0)
            {

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button - 1);

                    if (currentbutton != nullptr)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadUp, currentbutton);
                    }
                }
            }
        }
    } else
    {

        for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
        {
            SetJoystick *currentset = set.value();
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
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        } else if (ui->vdpadLeftComboBox->currentIndex() == index)
        {
            ui->vdpadLeftComboBox->setCurrentIndex(0);
        } else if (ui->vdpadRightComboBox->currentIndex() == index)
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

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis - 1);
                    JoyButton *currentbutton = nullptr;

                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    } else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadDown, currentbutton);
                }
            } else if (button > 0)
            {

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button - 1);

                    if (currentbutton != nullptr)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadDown, currentbutton);
                    }
                }
            }
        }
    } else
    {

        for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
        {
            SetJoystick *currentset = set.value();
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
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        } else if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        } else if (ui->vdpadRightComboBox->currentIndex() == index)
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

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis - 1);
                    JoyButton *currentbutton = nullptr;

                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    } else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadLeft, currentbutton);
                }
            } else if (button > 0)
            {

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button - 1);

                    if (currentbutton != nullptr)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadLeft, currentbutton);
                    }
                }
            }
        }
    } else
    {

        for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
        {
            SetJoystick *currentset = set.value();
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
    if (index > 0)
    {
        if (ui->vdpadUpComboBox->currentIndex() == index)
        {
            ui->vdpadUpComboBox->setCurrentIndex(0);
        } else if (ui->vdpadDownComboBox->currentIndex() == index)
        {
            ui->vdpadDownComboBox->setCurrentIndex(0);
        } else if (ui->vdpadLeftComboBox->currentIndex() == index)
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

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyAxis *currentaxis = currentset->getJoyAxis(axis - 1);
                    JoyButton *currentbutton = nullptr;

                    if (button == 0)
                    {
                        currentbutton = currentaxis->getNAxisButton();
                    } else if (button == 1)
                    {
                        currentbutton = currentaxis->getPAxisButton();
                    }

                    vdpad->addVButton(JoyDPadButton::DpadRight, currentbutton);
                }
            } else if (button > 0)
            {

                for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
                {
                    SetJoystick *currentset = set.value();
                    VDPad *vdpad = currentset->getVDPad(0);
                    JoyButton *currentbutton = currentset->getJoyButton(button - 1);

                    if (currentbutton != nullptr)
                    {
                        vdpad->addVButton(JoyDPadButton::DpadRight, currentbutton);
                    }
                }
            }
        }
    } else
    {

        for (auto set = joystick->getJoystick_sets().begin(); set != joystick->getJoystick_sets().end(); ++set)
        {
            SetJoystick *currentset = set.value();
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
    connect(ui->vdpadUpComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceStickAssignmentDialog::changeVDPadUpButton);
    connect(ui->vdpadDownComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceStickAssignmentDialog::changeVDPadDownButton);
    connect(ui->vdpadLeftComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceStickAssignmentDialog::changeVDPadLeftButton);
    connect(ui->vdpadRightComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceStickAssignmentDialog::changeVDPadRightButton);
}

void AdvanceStickAssignmentDialog::disableVDPadComboBoxes()
{
    disconnect(ui->vdpadUpComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceStickAssignmentDialog::changeVDPadUpButton);
    disconnect(ui->vdpadDownComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceStickAssignmentDialog::changeVDPadDownButton);
    disconnect(ui->vdpadLeftComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceStickAssignmentDialog::changeVDPadLeftButton);
    disconnect(ui->vdpadRightComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceStickAssignmentDialog::changeVDPadRightButton);
}

void AdvanceStickAssignmentDialog::openQuickAssignDialogStick1()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Move stick 1 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis != nullptr)
        {
            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this,
                    [this, axis] { ui->xAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex()); });
        }
    }

    msgBox.exec();
    msgBox.setText(tr("Move stick 1 along the Y axis"));

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);

            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this,
                    [this, axis] { ui->yAxisOneComboBox->setCurrentIndex(axis->getRealJoyIndex()); });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
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
    QMessageBox msgBox;
    msgBox.setText(tr("Move stick 2 along the X axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis != nullptr)
        {
            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this,
                    [this, axis] { ui->xAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex()); });
        }
    }

    msgBox.exec();
    msgBox.setText(tr("Move stick 2 along the Y axis"));

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);
        if (axis != nullptr)
        {
            disconnect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            disconnect(axis, &JoyAxis::active, this, nullptr);

            connect(axis, &JoyAxis::active, &msgBox, &QMessageBox::close);
            connect(axis, &JoyAxis::active, this,
                    [this, axis] { ui->yAxisTwoComboBox->setCurrentIndex(axis->getRealJoyIndex()); });
        }
    }

    msgBox.exec();

    for (int i = 0; i < joystick->getNumberAxes(); i++)
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
    joystick->getActiveSetJoystick()->setIgnoreEventState(false);
    joystick->getActiveSetJoystick()->release();
}

void AdvanceStickAssignmentDialog::openAssignVDPadUp()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton *axbtn = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, axbtn] { quickAssignVDPadUp(axbtn); });

            JoyAxisButton *axbtnp = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, axbtnp] { quickAssignVDPadUp(axbtnp); });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);

        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button]() { quickAssignVDPadUpBtn(button); });
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
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton *axbtn = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this, [this, axbtn] { quickAssignVDPadDown(axbtn); });

            JoyAxisButton *axbtnP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this, [this, axbtnP] { quickAssignVDPadDown(axbtnP); });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);

        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] { quickAssignVDPadDownJbtn(button); });
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
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton *joyaxisN = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this,
                    [this, joyaxisN]() { quickAssignVDPadLeft(joyaxisN); });

            JoyAxisButton *joyaxisP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this,
                    [this, joyaxisP]() { quickAssignVDPadLeft(joyaxisP); });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);

        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] { quickAssignVDPadLeftJbtn(button); });
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
    QMessageBox msgBox;
    msgBox.setText(tr("Press a button or move an axis"));
    msgBox.setStandardButtons(QMessageBox::Close);

    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if ((axis != nullptr) && !axis->isPartControlStick())
        {
            JoyAxisButton *joyaxisbtnN = axis->getNAxisButton();
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getNAxisButton(), &JoyAxisButton::clicked, this,
                    [this, joyaxisbtnN] { quickAssignVDPadRight(joyaxisbtnN); });

            JoyAxisButton *joyaxisbtnP = axis->getPAxisButton();
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, &msgBox, &QMessageBox::close);
            connect(axis->getPAxisButton(), &JoyAxisButton::clicked, this,
                    [this, joyaxisbtnP] { quickAssignVDPadRight(joyaxisbtnP); });
        }
    }

    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);

        if (button != nullptr)
        {
            connect(button, &JoyButton::clicked, &msgBox, &QMessageBox::close);
            connect(button, &JoyButton::clicked, this, [this, button] { quickAssignVDPadRightJbtn(button); });
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

void AdvanceStickAssignmentDialog::quickAssignVDPadUp(JoyAxisButton *joyaxisbtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));

    if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
        templist.append(QVariant(0));
    else
        templist.append(QVariant(1));

    int index = ui->vdpadUpComboBox->findData(templist);

    if (index > 0)
    {
        ui->vdpadUpComboBox->setCurrentIndex(index);
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadUpBtn(JoyButton *joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber() + 1));

    int index = ui->vdpadUpComboBox->findData(templist);

    if (index > 0)
    {
        ui->vdpadUpComboBox->setCurrentIndex(index);
    }
}

void AdvanceStickAssignmentDialog::quickAssignVDPadDown(JoyAxisButton *axbtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(axbtn->getAxis()->getRealJoyIndex()));

    if (axbtn->getAxis()->getNAxisButton() == axbtn)
        templist.append(QVariant(0));
    else
        templist.append(QVariant(1));

    int index = ui->vdpadDownComboBox->findData(templist);

    if (index > 0)
        ui->vdpadDownComboBox->setCurrentIndex(index);
}

void AdvanceStickAssignmentDialog::quickAssignVDPadDownJbtn(JoyButton *joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber() + 1));

    int index = ui->vdpadDownComboBox->findData(templist);

    if (index > 0)
        ui->vdpadDownComboBox->setCurrentIndex(index);
}

void AdvanceStickAssignmentDialog::quickAssignVDPadLeft(JoyAxisButton *joyaxisbtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));

    if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
        templist.append(QVariant(0));
    else
        templist.append(QVariant(1));

    int index = ui->vdpadLeftComboBox->findData(templist);

    if (index > 0)
        ui->vdpadLeftComboBox->setCurrentIndex(index);
}

void AdvanceStickAssignmentDialog::quickAssignVDPadLeftJbtn(JoyButton *joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber() + 1));

    int index = ui->vdpadLeftComboBox->findData(templist);

    if (index > 0)
        ui->vdpadLeftComboBox->setCurrentIndex(index);
}

void AdvanceStickAssignmentDialog::quickAssignVDPadRight(JoyAxisButton *joyaxisbtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(joyaxisbtn->getAxis()->getRealJoyIndex()));

    if (joyaxisbtn->getAxis()->getNAxisButton() == joyaxisbtn)
        templist.append(QVariant(0));
    else
        templist.append(QVariant(1));

    int index = ui->vdpadRightComboBox->findData(templist);

    if (index > 0)
        ui->vdpadRightComboBox->setCurrentIndex(index);
}

void AdvanceStickAssignmentDialog::quickAssignVDPadRightJbtn(JoyButton *joybtn)
{
    QList<QVariant> templist;
    templist.append(QVariant(0));
    templist.append(QVariant(joybtn->getJoyNumber() + 1));

    int index = ui->vdpadRightComboBox->findData(templist);

    if (index > 0)
        ui->vdpadRightComboBox->setCurrentIndex(index);
}

Joystick *AdvanceStickAssignmentDialog::getJoystick() const { return joystick; }
