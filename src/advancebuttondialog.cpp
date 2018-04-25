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

#include "advancebuttondialog.h"
#include "ui_advancebuttondialog.h"
#include "event.h"
#include "inputdevice.h"
#include "joybutton.h"
#include "simplekeygrabberbutton.h"
//#include "logger.h"

#include <cmath>

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QListWidgetItem>


const int AdvanceButtonDialog::MINIMUMTURBO = 2;

AdvanceButtonDialog::AdvanceButtonDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AdvanceButtonDialog),
    helper(button)
{
    ui->setupUi(this);

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    setAttribute(Qt::WA_DeleteOnClose);

    PadderCommon::inputDaemonMutex.lock();

    this->button = button;
    oldRow = 0;

    helper.moveToThread(button->thread());

    if (this->button->getToggleState())
    {
        ui->toggleCheckbox->setChecked(true);
    }

    if (this->button->isUsingTurbo())
    {
        ui->turboCheckbox->setChecked(true);
        ui->turboSlider->setEnabled(true);
    }

    int interval = this->button->getTurboInterval() / 10;
    if (interval < MINIMUMTURBO)
    {
        interval = JoyButton::ENABLEDTURBODEFAULT / 10;
    }
    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot*> iter(*(this->button->getAssignedSlots()));
    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        existingCode->setText(buttonslot->getSlotString());
        if (buttonslot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
        {
            if (!buttonslot->getTextData().isEmpty())
            {
                existingCode->setValue(buttonslot->getTextData(), JoyButtonSlot::JoyLoadProfile);
                existingCode->setToolTip(buttonslot->getTextData());
            }
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyTextEntry)
        {
            if (!buttonslot->getTextData().isEmpty())
            {
                existingCode->setValue(buttonslot->getTextData(), JoyButtonSlot::JoyTextEntry);
                existingCode->setToolTip(buttonslot->getTextData());
            }
        }
        else if (buttonslot->getSlotMode() == JoyButtonSlot::JoyExecute)
        {
            if (!buttonslot->getTextData().isEmpty())
            {
                existingCode->setValue(buttonslot->getTextData(), JoyButtonSlot::JoyExecute);
                existingCode->setToolTip(buttonslot->getTextData());

                if (buttonslot->getExtraData().canConvert<QString>())
                {
                    QString argumentsTemp = buttonslot->getExtraData().toString();
                    existingCode->getValue()->setExtraData(argumentsTemp);
                }
            }
        }
        else
        {
            existingCode->setValue(buttonslot->getSlotCode(),
                                   buttonslot->getSlotCodeAlias(),
                                   buttonslot->getSlotMode());
        }

        QListWidgetItem *item = new QListWidgetItem(ui->slotListWidget);
        item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton*>(existingCode));
        QHBoxLayout *layout= new QHBoxLayout();
        layout->setContentsMargins(10, 0, 10, 0);
        layout->addWidget(existingCode);
        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        item->setSizeHint(widget->sizeHint());

        ui->slotListWidget->setItemWidget(item, widget);
        connectButtonEvents(existingCode);
    }

    appendBlankKeyGrabber();
    populateSetSelectionComboBox();
    populateSlotSetSelectionComboBox();

    if ((this->button->getSetSelection() > -1) &&
        (this->button->getChangeSetCondition() != JoyButton::SetChangeDisabled))
    {
        int selectIndex = static_cast<int>(this->button->getChangeSetCondition());
        selectIndex += this->button->getSetSelection() * 3;
        if (this->button->getOriginSet() < this->button->getSetSelection())
        {
            selectIndex -= 3;
        }

        ui->setSelectionComboBox->setCurrentIndex(selectIndex);
    }

    fillTimeComboBoxes();
    ui->actionTenthsComboBox->setCurrentIndex(1);

    updateActionTimeLabel();
    changeTurboForSequences();

    if (button->isCycleResetActive())
    {
        ui->autoResetCycleCheckBox->setEnabled(true);
        ui->autoResetCycleCheckBox->setChecked(true);
        checkCycleResetWidgetStatus(true);
    }

    if (button->getCycleResetTime() != 0)
    {
        populateAutoResetInterval();
    }

    updateWindowTitleButtonName();

    if (this->button->isPartRealAxis() && this->button->isUsingTurbo())
    {
        ui->turboModeComboBox->setEnabled(true);
    }
    else if (!this->button->isPartRealAxis())
    {
        ui->turboModeComboBox->setVisible(false);
        ui->turboModeLabel->setVisible(false);
    }

    findTurboModeComboIndex();

    // Don't show Set Selector page for modifier buttons
    if (this->button->isModifierButton())
    {
        delete ui->listWidget->item(3);
    }

    //performStatsWidgetRefresh(ui->slotListWidget->currentItem());
    changeSlotHelpText(ui->slotTypeComboBox->currentIndex());

    PadderCommon::inputDaemonMutex.unlock();

    ui->resetCycleDoubleSpinBox->setMaximum(JoyButton::MAXCYCLERESETTIME * 0.001); // static_cast<double>

    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), ui->turboSlider, SLOT(setEnabled(bool)));
    connect(ui->turboSlider, SIGNAL(valueChanged(int)), this, SLOT(checkTurboIntervalValue(int)));

    connect(ui->insertSlotButton, SIGNAL(clicked()), this, SLOT(insertSlot()));
    connect(ui->deleteSlotButton, SIGNAL(clicked()), this, SLOT(deleteSlot()));
    connect(ui->clearAllPushButton, SIGNAL(clicked()), this, SLOT(clearAllSlots()));

    connect(ui->slotTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSlotTypeDisplay(int)));
    connect(ui->slotTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSlotHelpText(int)));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateActionTimeLabel()));

    connect(ui->toggleCheckbox, SIGNAL(clicked(bool)), button, SLOT(setToggle(bool)));
    connect(ui->turboCheckbox, SIGNAL(clicked(bool)), this, SLOT(checkTurboSetting(bool)));

    connect(ui->setSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSetSelection()));

    connect(ui->slotListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(performStatsWidgetRefresh(QListWidgetItem*)));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotTimeUpdate()));
    connect(ui->slotSetChangeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkSlotSetChangeUpdate()));

    connect(ui->distanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotDistanceUpdate()));
    connect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkSlotMouseModUpdate()));

    connect(ui->autoResetCycleCheckBox, SIGNAL(clicked(bool)), this, SLOT(checkCycleResetWidgetStatus(bool)));
    connect(ui->autoResetCycleCheckBox, SIGNAL(clicked(bool)), this, SLOT(setButtonCycleReset(bool)));
    connect(ui->resetCycleDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setButtonCycleResetInterval(double)));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckbox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
    connect(ui->turboModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setButtonTurboMode(int)));
    connect(ui->loadProfilePushButton, SIGNAL(clicked()), this, SLOT(showSelectProfileWindow()));
    connect(ui->execToolButton, SIGNAL(clicked(bool)), this, SLOT(showFindExecutableWindow(bool)));
}

AdvanceButtonDialog::~AdvanceButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    delete ui;
}

void AdvanceButtonDialog::changeTurboText(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (value >= MINIMUMTURBO)
    {
        double delay = value / 100.0;
        double clicks = 100.0 / (double)value;
        QString delaytext = QString::number(delay, 'g', 3).append(" ").append(trUtf8("sec."));
        QString labeltext = QString::number(clicks, 'g', 2).append(" ").append(trUtf8("/sec."));

        ui->delayValueLabel->setText(delaytext);
        ui->rateValueLabel->setText(labeltext);
    }
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    if ((index == (itemcount - 1)) && (value >= 0))
    {
        // New slot added on the old blank button. Append
        // new blank button to the end of the list.
        appendBlankKeyGrabber();
    }

    // Go through all grabber buttons in list and possibly resize widgets.
    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->slotListWidget->item(i);
        QWidget *widget = ui->slotListWidget->itemWidget(item);
        item->setSizeHint(widget->sizeHint());
    }

    // Alter interface if turbo cannot be used.
    changeTurboForSequences();

    emit slotsChanged();
}

void AdvanceButtonDialog::connectButtonEvents(SimpleKeyGrabberButton *button)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(button, SIGNAL(clicked()), this, SLOT(changeSelectedSlot()));
    connect(button, SIGNAL(buttonCodeChanged(int)), this, SLOT(updateSelectedSlot(int)));
}

void AdvanceButtonDialog::updateSelectedSlot(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SimpleKeyGrabberButton *grabbutton = qobject_cast<SimpleKeyGrabberButton*>(sender()); // static_cast
    JoyButtonSlot *tempbuttonslot = grabbutton->getValue();
    int index = ui->slotListWidget->currentRow();

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, tempbuttonslot->getSlotCode()),
                              Q_ARG(int, tempbuttonslot->getSlotCodeAlias()),
                              Q_ARG(int, index),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

    updateSlotsScrollArea(value);
}

void AdvanceButtonDialog::deleteSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    QListWidgetItem *item = ui->slotListWidget->takeItem(index);
    delete item;
    item = 0;

    // Deleted last button. Replace with new blank button
    if (index == (itemcount - 1))
    {
        appendBlankKeyGrabber();
    }

    changeTurboForSequences();

    QMetaObject::invokeMethod(&helper, "removeAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, index));

    index = qMax(0, index-1);
    performStatsWidgetRefresh(ui->slotListWidget->item(index));

    emit slotsChanged();
}

void AdvanceButtonDialog::changeSelectedSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SimpleKeyGrabberButton *button = qobject_cast<SimpleKeyGrabberButton*>(sender()); // static_cast

    bool leave = false;
    for (int i = 0; (i < ui->slotListWidget->count()) && !leave; i++)
    {
        QListWidgetItem *item = ui->slotListWidget->item(i);
        SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole)
                .value<SimpleKeyGrabberButton*>();

        if (button == tempbutton)
        {
            ui->slotListWidget->setCurrentRow(i);
            leave = true;
            oldRow = i;
        }
    }
}

void AdvanceButtonDialog::appendBlankKeyGrabber()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
    QListWidgetItem *item = new QListWidgetItem(ui->slotListWidget);
    item->setData(Qt::UserRole,
                  QVariant::fromValue<SimpleKeyGrabberButton*>(blankButton));

    QHBoxLayout *layout= new QHBoxLayout();
    layout->setContentsMargins(10, 0, 10, 0);
    layout->addWidget(blankButton);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    item->setSizeHint(widget->sizeHint());

    ui->slotListWidget->setItemWidget(item, widget);
    ui->slotListWidget->setCurrentItem(item);
    connectButtonEvents(blankButton);
    ui->slotTypeComboBox->setCurrentIndex(KBMouseSlot);
}

void AdvanceButtonDialog::insertSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int current = ui->slotListWidget->currentRow();
    int count = ui->slotListWidget->count();
    int slotTypeIndex = ui->slotTypeComboBox->currentIndex();

    if (slotTypeIndex == KBMouseSlot)
    {
        if (current != (count - 1))
        {
            SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
            QListWidgetItem *item = new QListWidgetItem();
            ui->slotListWidget->insertItem(current, item);
            item->setData(Qt::UserRole,
                          QVariant::fromValue<SimpleKeyGrabberButton*>(blankButton));

            QHBoxLayout *layout= new QHBoxLayout();
            layout->addWidget(blankButton);
            QWidget *widget = new QWidget();
            widget->setLayout(layout);
            item->setSizeHint(widget->sizeHint());
            ui->slotListWidget->setItemWidget(item, widget);
            ui->slotListWidget->setCurrentItem(item);
            connectButtonEvents(blankButton);
            blankButton->refreshButtonLabel();

            QMetaObject::invokeMethod(&helper, "insertAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, 0), Q_ARG(uint, 0),
                                      Q_ARG(int, current));

            updateSlotsScrollArea(0);
        }
    }
    else if (slotTypeIndex == CycleSlot)
    {
        insertCycleSlot();
    }
    else if (slotTypeIndex == DelaySlot)
    {
        insertDelaySlot();
    }
    else if (slotTypeIndex == DistanceSlot)
    {
        insertDistanceSlot();
    }
    else if (slotTypeIndex == HoldSlot)
    {
        insertHoldSlot();
    }
    else if (slotTypeIndex == LoadSlot)
    {
        showSelectProfileWindow();
    }
    else if (slotTypeIndex == MouseModSlot)
    {
        insertMouseSpeedModSlot();
    }
    else if (slotTypeIndex == PauseSlot)
    {
        insertPauseSlot();
    }
    else if (slotTypeIndex == PressTimeSlot)
    {
        insertKeyPressSlot();
    }
    else if (slotTypeIndex == ReleaseSlot)
    {
        insertReleaseSlot();
    }
    else if (slotTypeIndex == SetChangeSlot)
    {
        insertSetChangeSlot();
    }
    else if (slotTypeIndex == TextEntry)
    {
        insertTextEntrySlot();
    }
    else if (slotTypeIndex == ExecuteSlot)
    {
        insertExecuteSlot();
    }
}

void AdvanceButtonDialog::insertPauseSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime >= 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyPause);
        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, actionTime),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyPause));

        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertReleaseSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime >= 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyRelease);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, actionTime),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyRelease));

        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertHoldSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyHold);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, actionTime),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyHold));

        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertSetChangeSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();

    int currentIndex = ui->slotSetChangeComboBox->currentIndex();
    int setIndex = ui->slotSetChangeComboBox->itemData(currentIndex).toInt();
    if (setIndex >= 0)
    {
        tempbutton->setValue(setIndex, JoyButtonSlot::JoySetChange);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, setIndex),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoySetChange));

        updateSlotsScrollArea(setIndex);
    }
}

int AdvanceButtonDialog::actionTimeConvert()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int minutesIndex = ui->actionMinutesComboBox->currentIndex();
    int secondsIndex = ui->actionSecondsComboBox->currentIndex();
    int hundredthsIndex = ui->actionHundredthsComboBox->currentIndex();
    int tenthsIndex = ui->actionTenthsComboBox->currentIndex();

    int tempMilliSeconds = minutesIndex * 1000 * 60;
    tempMilliSeconds += secondsIndex * 1000;
    tempMilliSeconds += tenthsIndex * 100;
    tempMilliSeconds += hundredthsIndex * 10;
    return tempMilliSeconds;
}

void AdvanceButtonDialog::refreshTimeComboBoxes(JoyButtonSlot *slot)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnectTimeBoxesEvents();

    int slottime = slot->getSlotCode();
    int tempMinutes = slottime / 1000 / 60;
    int tempSeconds = slottime / 1000 % 60;
    int tempTenthsSeconds = (slottime % 1000) / 100;
    int tempHundredthsSeconds = (slottime % 1000 % 100) / 10;

    ui->actionMinutesComboBox->setCurrentIndex(tempMinutes);
    ui->actionSecondsComboBox->setCurrentIndex(tempSeconds);
    ui->actionTenthsComboBox->setCurrentIndex(tempTenthsSeconds);
    ui->actionHundredthsComboBox->setCurrentIndex(tempHundredthsSeconds);
    updateActionTimeLabel();

    connectTimeBoxesEvents();
}

void AdvanceButtonDialog::updateActionTimeLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int actionTime = actionTimeConvert();
    int minutes = actionTime / 1000 / 60;
    double hundredths = actionTime % 1000 / 1000.0;
    double seconds = (actionTime / 1000 % 60) + hundredths;
    QString temp = QString();
    temp.append(QString::number(minutes)).append("m ");
    temp.append(QString::number(seconds, 'f', 2)).append("s");
    ui->actionTimeLabel->setText(temp);
}

void AdvanceButtonDialog::clearAllSlots()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->slotListWidget->clear();
    appendBlankKeyGrabber();
    changeTurboForSequences();

    QMetaObject::invokeMethod(button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
    performStatsWidgetRefresh(ui->slotListWidget->currentItem());

    emit slotsChanged();
}

void AdvanceButtonDialog::changeTurboForSequences()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool containsSequences = false;
    for (int i = 0; (i < ui->slotListWidget->count()) && !containsSequences; i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)
                ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if ((tempbuttonslot->getSlotCode() > 0) &&
            ((tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyPause) ||
             (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyHold) ||
             (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
            )
           )
        {
            containsSequences = true;
        }
    }

    if (containsSequences)
    {
        if (ui->turboCheckbox->isChecked())
        {
            ui->turboCheckbox->setChecked(false);
            this->button->setUseTurbo(false);
            emit turboChanged(false);
        }

        if (ui->turboCheckbox->isEnabled())
        {
            ui->turboCheckbox->setEnabled(false);
            emit turboButtonEnabledChange(false);
        }
    }
    else
    {
        if (!ui->turboCheckbox->isEnabled())
        {
            ui->turboCheckbox->setEnabled(true);
            emit turboButtonEnabledChange(true);
        }
    }
}

void AdvanceButtonDialog::insertCycleSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(1, JoyButtonSlot::JoyCycle);

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, 1),
                              Q_ARG(int, 0),
                              Q_ARG(int, index),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyCycle));

    updateSlotsScrollArea(1);
}

void AdvanceButtonDialog::insertDistanceSlot()
{

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();

    int tempDistance = 0;
    for (int i = 0; i < ui->slotListWidget->count(); i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)
                ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        JoyButtonSlot *tempbuttonslot = button->getValue();
        if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
        {
            tempDistance += tempbuttonslot->getSlotCode();
        }
        else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            tempDistance = 0;
        }
    }

    int testDistance = ui->distanceSpinBox->value();
    if ((testDistance + tempDistance) <= 100)
    {
        tempbutton->setValue(testDistance, JoyButtonSlot::JoyDistance);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, testDistance),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyDistance));

        updateSlotsScrollArea(testDistance);
    }
}

void AdvanceButtonDialog::placeNewSlot(JoyButtonSlot *slot)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    tempbutton->setValue(slot->getSlotCode(), slot->getSlotCodeAlias(), slot->getSlotMode());

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                              Q_ARG(int, slot->getSlotCode()),
                              Q_ARG(int, slot->getSlotCodeAlias()),
                              Q_ARG(int, index),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, slot->getSlotMode()));

    updateSlotsScrollArea(slot->getSlotCode());

    slot->deleteLater();
}

void AdvanceButtonDialog::updateTurboIntervalValue(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (value >= MINIMUMTURBO)
    {
        button->setTurboInterval(value * 10);
    }
}

void AdvanceButtonDialog::checkTurboSetting(bool state)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->turboCheckbox->setChecked(state);
    ui->turboSlider->setEnabled(state);

    if (this->button->isPartRealAxis())
    {
        ui->turboModeComboBox->setEnabled(state);
    }

    changeTurboForSequences();
    button->setUseTurbo(state);
    if ((button->getTurboInterval() / 10) >= MINIMUMTURBO)
    {
        ui->turboSlider->setValue(button->getTurboInterval() / 10);
    }
}

void AdvanceButtonDialog::updateSetSelection()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    PadderCommon::inputDaemonMutex.lock();

    int condition_choice = 0;
    int chosen_set = -1;
    JoyButton::SetChangeCondition set_selection_condition = JoyButton::SetChangeDisabled;

    if (ui->setSelectionComboBox->currentIndex() > 0)
    {
        condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;
        chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;

        // Above removed rows
        if (button->getOriginSet() > chosen_set)
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
        }
        // Below removed rows
        else
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() + 2) / 3;
        }

        qDebug() << "CONDITION: " << QString::number(condition_choice) << endl;
        if (condition_choice == 0)
        {
            set_selection_condition = JoyButton::SetChangeOneWay;
        }
        else if (condition_choice == 1)
        {
            set_selection_condition = JoyButton::SetChangeTwoWay;
        }
        else if (condition_choice == 2)
        {
            set_selection_condition = JoyButton::SetChangeWhileHeld;
        }

        qDebug() << "CHOSEN SET: " << chosen_set << endl;
    }
    else
    {
        chosen_set = -1;
        set_selection_condition = JoyButton::SetChangeDisabled;
    }

    if ((chosen_set > -1) && (set_selection_condition != JoyButton::SetChangeDisabled))
    {
        // First, remove old condition for the button in both sets.
        // After that, make the new assignment.
        button->setChangeSetCondition(JoyButton::SetChangeDisabled);

        button->setChangeSetSelection(chosen_set);
        button->setChangeSetCondition(set_selection_condition);
    }
    else
    {
        button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void AdvanceButtonDialog::checkTurboIntervalValue(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (value >= MINIMUMTURBO)
    {
        changeTurboText(value);
        updateTurboIntervalValue(value);
    }
    else
    {
        ui->turboSlider->setValue(MINIMUMTURBO);
    }
}

void AdvanceButtonDialog::fillTimeComboBoxes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->actionMinutesComboBox->clear();
    ui->actionSecondsComboBox->clear();
    ui->actionHundredthsComboBox->clear();
    ui->actionTenthsComboBox->clear();

    for (double i=0; i <= 10; i++)
    {
        QString temp = QString::number(i, 'g', 2).append("m");
        ui->actionMinutesComboBox->addItem(temp);
    }

    for (double i=0; i <= 59; i++)
    {
        QString temp = QString::number(i, 'g', 2);
        ui->actionSecondsComboBox->addItem(temp);
    }

    for (int i=0; i < 10; i++)
    {
        QString temp = QString(".%1").arg(i, 1, 10, QChar('0'));
        ui->actionTenthsComboBox->addItem(temp);
    }

    for (int i=0; i < 10; i++)
    {
        QString temp = QString("%1s").arg(i, 1, 10, QChar('0'));
        ui->actionHundredthsComboBox->addItem(temp);
    }
}

void AdvanceButtonDialog::insertMouseSpeedModSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int tempMouseMod = ui->mouseSpeedModSpinBox->value();
    if (tempMouseMod > 0)
    {
        tempbutton->setValue(tempMouseMod, JoyButtonSlot::JoyMouseSpeedMod);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, tempMouseMod),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyMouseSpeedMod));

        updateSlotsScrollArea(tempMouseMod);
    }
}

void AdvanceButtonDialog::insertKeyPressSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyKeyPress);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, actionTime),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyKeyPress));

        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertDelaySlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    int actionTime = actionTimeConvert();
    if (actionTime > 0)
    {
        tempbutton->setValue(actionTime, JoyButtonSlot::JoyDelay);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, actionTime),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyDelay));

        updateSlotsScrollArea(actionTime);
    }
}

void AdvanceButtonDialog::insertTextEntrySlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    QString temp = ui->textEntryLineEdit->text();
    if (!temp.isEmpty())
    {
        tempbutton->setValue(temp, JoyButtonSlot::JoyTextEntry);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot*, tempbutton->getValue()),
                                  Q_ARG(int, index));
        tempbutton->setToolTip(temp);
        updateSlotsScrollArea(0);
    }
}

void AdvanceButtonDialog::insertExecuteSlot()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    QString temp = ui->execLineEdit->text();
    QString argumentsTemp = ui->execArgumentsLineEdit->text();
    if (!temp.isEmpty())
    {
        QFileInfo tempFileInfo(temp);
        if (tempFileInfo.exists() && tempFileInfo.isExecutable())
        {
            tempbutton->setValue(temp, JoyButtonSlot::JoyExecute);
            if (!argumentsTemp.isEmpty())
            {
                tempbutton->getValue()->setExtraData(QVariant(argumentsTemp));
            }

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(JoyButtonSlot*, tempbutton->getValue()),
                                      Q_ARG(int, index));

            tempbutton->setToolTip(temp);
            updateSlotsScrollArea(0);
        }
    }
}

void AdvanceButtonDialog::performStatsWidgetRefresh(QListWidgetItem *item)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *slot = tempbutton->getValue();

    if ((slot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (slot->getSlotCode() != 0))
    {
        ui->slotTypeComboBox->setCurrentIndex(KBMouseSlot);
    }
    else if ((slot->getSlotMode() == JoyButtonSlot::JoyMouseButton) ||
        (slot->getSlotMode() == JoyButtonSlot::JoyMouseMovement))
    {
        ui->slotTypeComboBox->setCurrentIndex(KBMouseSlot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
    {
        ui->slotTypeComboBox->setCurrentIndex(CycleSlot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDelay)
    {
        ui->slotTypeComboBox->setCurrentIndex(DelaySlot);

        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        ui->slotTypeComboBox->setCurrentIndex(DistanceSlot);

        disconnect(ui->distanceSpinBox, SIGNAL(valueChanged(int)),
                   this, SLOT(checkSlotDistanceUpdate()));
        ui->distanceSpinBox->setValue(slot->getSlotCode());
        connect(ui->distanceSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(checkSlotDistanceUpdate()));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        ui->slotTypeComboBox->setCurrentIndex(HoldSlot);
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        ui->slotTypeComboBox->setCurrentIndex(LoadSlot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod)
    {
        ui->slotTypeComboBox->setCurrentIndex(MouseModSlot);
        disconnect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)),
                   this, SLOT(checkSlotMouseModUpdate()));
        ui->mouseSpeedModSpinBox->setValue(slot->getSlotCode());
        connect(ui->mouseSpeedModSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(checkSlotMouseModUpdate()));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyPause)
    {
        ui->slotTypeComboBox->setCurrentIndex(PauseSlot);
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyKeyPress)
    {
        ui->slotTypeComboBox->setCurrentIndex(PressTimeSlot);
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyRelease)
    {
        ui->slotTypeComboBox->setCurrentIndex(ReleaseSlot);
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoySetChange)
    {
        disconnect(ui->slotSetChangeComboBox, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(checkSlotSetChangeUpdate()));

        ui->slotTypeComboBox->setCurrentIndex(SetChangeSlot);
        int chooseIndex = slot->getSlotCode();

        int foundIndex = ui->slotSetChangeComboBox->findData(QVariant(chooseIndex));
        if (foundIndex >= 0)
        {
            ui->slotSetChangeComboBox->setCurrentIndex(foundIndex);
        }

        connect(ui->slotSetChangeComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(checkSlotSetChangeUpdate()));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyTextEntry)
    {
        ui->slotTypeComboBox->setCurrentIndex(TextEntry);
        ui->textEntryLineEdit->setText(slot->getTextData());
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyExecute)
    {
        ui->slotTypeComboBox->setCurrentIndex(ExecuteSlot);
        ui->execLineEdit->setText(slot->getTextData());
        ui->execArgumentsLineEdit->setText(slot->getExtraData().toString());
    }
}

void AdvanceButtonDialog::checkSlotTimeUpdate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *tempbuttonslot = tempbutton->getValue();
    if ((tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyPause) ||
        (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyHold) ||
        (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyRelease) ||
        (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyKeyPress) ||
        (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDelay))
    {
        int actionTime = actionTimeConvert();
        if (actionTime > 0)
        {
            tempbutton->setValue(actionTime, tempbuttonslot->getSlotMode());

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, actionTime),
                                      Q_ARG(int, 0),
                                      Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

            updateSlotsScrollArea(actionTime);
        }
    }

}

void AdvanceButtonDialog::checkSlotMouseModUpdate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *tempbuttonslot = tempbutton->getValue();

    int tempMouseMod = ui->mouseSpeedModSpinBox->value();
    if ((tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod) &&
        (tempMouseMod > 0))
    {
        tempbutton->setValue(tempMouseMod, tempbuttonslot->getSlotMode());

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, tempMouseMod),
                                  Q_ARG(int, 0),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

        updateSlotsScrollArea(tempMouseMod);
    }
}

void AdvanceButtonDialog::checkSlotSetChangeUpdate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();
    if (buttonslot->getSlotMode() == JoyButtonSlot::JoySetChange)
    {
        int comboIndex = ui->slotSetChangeComboBox->currentIndex();
        int setIndex = ui->slotSetChangeComboBox->itemData(comboIndex).toInt();

        if (setIndex >= 0)
        {
            tempbutton->setValue(setIndex, buttonslot->getSlotMode());

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, setIndex),
                                      Q_ARG(int, 0),
                                      Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, buttonslot->getSlotMode()));

            updateSlotsScrollArea(setIndex);
        }
    }
}

void AdvanceButtonDialog::checkSlotDistanceUpdate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();
    int tempDistance = 0;

    if (buttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        for (int i = 0; i < ui->slotListWidget->count(); i++)
        {
            SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)
                    ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
            JoyButtonSlot *tempbuttonslot = button->getValue();
            if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
            {
                tempDistance += tempbuttonslot->getSlotCode();
            }
            else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
            {
                tempDistance = 0;
            }
        }

        int testDistance = ui->distanceSpinBox->value();
        tempDistance += testDistance - buttonslot->getSlotCode();
        if (tempDistance <= 100)
        {
            tempbutton->setValue(testDistance, buttonslot->getSlotMode());

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, testDistance),
                                      Q_ARG(int, 0),
                                      Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, buttonslot->getSlotMode()));

            updateSlotsScrollArea(testDistance);
        }
    }
}

void AdvanceButtonDialog::updateWindowTitleButtonName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    temp.append(trUtf8("Advanced").append(": ")).append(button->getPartialName(false, true));

    if (button->getParentSet()->getIndex() != 0)
    {
        int setIndex = button->getParentSet()->getRealIndex();
        temp.append(" [").append(trUtf8("Set %1").arg(setIndex));

        QString setName = button->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void AdvanceButtonDialog::checkCycleResetWidgetStatus(bool enabled)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (enabled)
    {
        ui->resetCycleDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->resetCycleDoubleSpinBox->setEnabled(false);
    }
}

void AdvanceButtonDialog::setButtonCycleResetInterval(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int milliseconds = (static_cast<int>(value) * 1000) + (fmod(value, 1.0) * 1000);
    button->setCycleResetTime(milliseconds);
}

void AdvanceButtonDialog::populateAutoResetInterval()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    double seconds = button->getCycleResetTime() / 1000.0;
    ui->resetCycleDoubleSpinBox->setValue(seconds);
}

void AdvanceButtonDialog::setButtonCycleReset(bool enabled)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (enabled)
    {
        button->setCycleResetStatus(true);
        if ((button->getCycleResetTime() == 0) && (ui->resetCycleDoubleSpinBox->value() > 0.0))
        {
            double current = ui->resetCycleDoubleSpinBox->value();
            setButtonCycleResetInterval(current);
        }
    }
    else
    {
        button->setCycleResetStatus(false);
    }
}

void AdvanceButtonDialog::resetTimeBoxes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnectTimeBoxesEvents();

    ui->actionMinutesComboBox->setCurrentIndex(0);
    ui->actionSecondsComboBox->setCurrentIndex(0);
    ui->actionTenthsComboBox->setCurrentIndex(1);
    ui->actionHundredthsComboBox->setCurrentIndex(0);

    updateActionTimeLabel();
    connectTimeBoxesEvents();
}

void AdvanceButtonDialog::disconnectTimeBoxesEvents()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(updateActionTimeLabel()));
    disconnect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(updateActionTimeLabel()));

    disconnect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(checkSlotTimeUpdate()));
    disconnect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(checkSlotTimeUpdate()));
}

void AdvanceButtonDialog::connectTimeBoxesEvents()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateActionTimeLabel()));
    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateActionTimeLabel()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateActionTimeLabel()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateActionTimeLabel()));

    connect(ui->actionHundredthsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionSecondsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionMinutesComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkSlotTimeUpdate()));
    connect(ui->actionTenthsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkSlotTimeUpdate()));
}

void AdvanceButtonDialog::populateSetSelectionComboBox()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->setSelectionComboBox->clear();
    ui->setSelectionComboBox->insertItem(0, trUtf8("Disabled"));

    int currentIndex = 1;
    for (int i = 0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        if (this->button->getOriginSet() != i)
        {
            QString temp = QString();
            temp.append(trUtf8("Select Set %1").arg(i+1));

            InputDevice *tempdevice = button->getParentSet()->getInputDevice();
            SetJoystick *tempset = tempdevice->getSetJoystick(i);
            if (tempset != nullptr)
            {
                QString setName = tempset->getName();
                if (!setName.isEmpty())
                {
                    temp.append(" ").append("[");
                    temp.append(setName).append("]").append(" ");
                }
            }

            QString oneWayText = QString();
            oneWayText.append(temp).append(" ").append(trUtf8("One Way"));

            QString twoWayText = QString();
            twoWayText.append(temp).append(" ").append(trUtf8("Two Way"));

            QString whileHeldText = QString();
            whileHeldText.append(temp).append(" ").append(trUtf8("While Held"));

            QStringList setChoices = QStringList();
            setChoices.append(oneWayText);
            setChoices.append(twoWayText);
            setChoices.append(whileHeldText);

            ui->setSelectionComboBox->insertItems(currentIndex, setChoices);
            currentIndex += 3;
        }
    }
}

void AdvanceButtonDialog::populateSlotSetSelectionComboBox()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    ui->slotSetChangeComboBox->clear();

    int currentIndex = 0;
    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        if (this->button->getOriginSet() != i)
        {
            QString temp = QString();
            temp.append(trUtf8("Select Set %1").arg(i+1));

            InputDevice *tempdevice = button->getParentSet()->getInputDevice();
            SetJoystick *tempset = tempdevice->getSetJoystick(i);
            if (tempset != nullptr)
            {
                QString setName = tempset->getName();
                if (!setName.isEmpty())
                {
                    temp.append(" ").append("[");
                    temp.append(setName).append("]").append(" ");
                }
            }

            ui->slotSetChangeComboBox->insertItem(currentIndex, temp, QVariant(i));
            currentIndex++;
        }
    }
}

void AdvanceButtonDialog::findTurboModeComboIndex()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::TurboMode currentTurboMode = this->button->getTurboMode();
    if (currentTurboMode == JoyButton::NormalTurbo)
    {
        ui->turboModeComboBox->setCurrentIndex(0);
    }
    else if (currentTurboMode == JoyButton::GradientTurbo)
    {
        ui->turboModeComboBox->setCurrentIndex(1);
    }
    else if (currentTurboMode == JoyButton::PulseTurbo)
    {
        ui->turboModeComboBox->setCurrentIndex(2);
    }
}

void AdvanceButtonDialog::setButtonTurboMode(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (value == 0)
    {
        this->button->setTurboMode(JoyButton::NormalTurbo);
    }
    else if (value == 1)
    {
        this->button->setTurboMode(JoyButton::GradientTurbo);
    }
    else if (value == 2)
    {
        this->button->setTurboMode(JoyButton::PulseTurbo);
    }
}

void AdvanceButtonDialog::showSelectProfileWindow()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    AntiMicroSettings *settings = this->button->getParentSet()->getInputDevice()->getSettings();

    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString filename = QFileDialog::getOpenFileName(this, trUtf8("Choose Profile"),
                                                    lookupDir, trUtf8("Config Files (*.amgp *.xml)"));
    if (!filename.isEmpty())
    {
        int index = ui->slotListWidget->currentRow();
        SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
                ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
        tempbutton->setValue(filename, JoyButtonSlot::JoyLoadProfile);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot*, tempbutton->getValue()),
                                  Q_ARG(int, index));

        tempbutton->setToolTip(filename);
        updateSlotsScrollArea(0);
    }
}

void AdvanceButtonDialog::showFindExecutableWindow(bool)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = ui->execLineEdit->text();
    QString lookupDir = QDir::homePath();
    if (!temp.isEmpty())
    {
        QFileInfo tempFileInfo(temp);
        if (tempFileInfo.absoluteDir().exists())
        {
            lookupDir = tempFileInfo.absoluteDir().absolutePath();
        }
    }

    QString filepath = QFileDialog::getOpenFileName(this, trUtf8("Choose Executable"), lookupDir);
    if (!filepath.isEmpty())
    {
        QFileInfo tempFileInfo(filepath);
        if (tempFileInfo.exists() && tempFileInfo.isExecutable())
        {
            ui->execLineEdit->setText(filepath);
        }
    }
}

void AdvanceButtonDialog::changeSlotTypeDisplay(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (index == KBMouseSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == CycleSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(3);
    }
    else if (index == DelaySlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == DistanceSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(2);
    }
    else if (index == HoldSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == LoadSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(4);
    }
    else if (index == MouseModSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(1);
    }
    else if (index == PauseSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == PressTimeSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == ReleaseSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(0);
    }
    else if (index == SetChangeSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(5);
    }
    else if (index == TextEntry)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(6);
    }
    else if (index == ExecuteSlot)
    {
        ui->slotControlsStackedWidget->setCurrentIndex(7);
    }
}

void AdvanceButtonDialog::changeSlotHelpText(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (index == KBMouseSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Insert a new blank slot."));
    }
    else if (index == CycleSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Slots past a Cycle action will be executed "
                                          "on the next button press. Multiple cycles can be added "
                                          "in order to create partitions in a sequence."));
    }
    else if (index == DelaySlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Delays the time that the next slot is activated "
                                          "by the time specified. Slots activated before the "
                                          "delay will remain active after the delay time "
                                          "has passed."));
    }
    else if (index == DistanceSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Distance action specifies that the slots afterwards "
                                          "will only be executed when an axis is moved "
                                          "a certain range past the designated dead zone."));
    }
    else if (index == HoldSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Insert a hold action. Slots after the action will only be "
                                          "executed if the button is held past the interval specified."));
    }
    else if (index == LoadSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Chose a profile to load when this slot is activated."));
    }
    else if (index == MouseModSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Mouse mod action will modify all mouse speed settings "
                                          "by a specified percentage while the action is being processed. "
                                          "This can be useful for slowing down the mouse while "
                                          "sniping."));
    }
    else if (index == PauseSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Insert a pause that occurs in between key presses."));
    }
    else if (index == PressTimeSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Specify the time that keys past this slot should be "
                                          "held down."));
    }
    else if (index == ReleaseSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Insert a release action. Slots after the action will only be "
                                          "executed after a button release if the button was held "
                                          "past the interval specified."));
    }
    else if (index == SetChangeSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Change to selected set once slot is activated."));
    }
    else if (index == TextEntry)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Full string will be typed when a "
                                          "slot is activated."));
    }
    else if (index == ExecuteSlot)
    {
        ui->slotTypeHelpLabel->setText(trUtf8("Execute program when slot is activated."));
    }
}

int AdvanceButtonDialog::getOldRow() const {

    return oldRow;
}

JoyButton *AdvanceButtonDialog::getButton() const {

    return button;
}

AdvanceButtonDialogHelper const& AdvanceButtonDialog::getHelper() {

    return helper;
}
