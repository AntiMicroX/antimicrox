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

#include "globalvariables.h"
#include "messagehandler.h"
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


AdvanceButtonDialog::AdvanceButtonDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AdvanceButtonDialog),
    helper(button)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);

    PadderCommon::inputDaemonMutex.lock();

    m_button = button;
    oldRow = 0;

    getHelperLocal().moveToThread(button->thread());

    if (m_button->getToggleState())
    {
        ui->toggleCheckbox->setChecked(true);
    }

    if (m_button->isUsingTurbo())
    {
        ui->turboCheckbox->setChecked(true);
        ui->turboSlider->setEnabled(true);
    }

    int interval = m_button->getTurboInterval() / 10;
    if (interval < GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        interval = GlobalVariables::JoyButton::ENABLEDTURBODEFAULT / 10;
    }
    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot*> iter(*(m_button->getAssignedSlots()));
    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        existingCode->setText(buttonslot->getSlotString());

        switch(static_cast<int>(buttonslot->getSlotMode()))
        {
            case 11:

                if (!buttonslot->getTextData().isEmpty())
                {
                    existingCode->setValue(buttonslot->getTextData(), JoyButtonSlot::JoyLoadProfile);
                    existingCode->setToolTip(buttonslot->getTextData());
                }

            break;

            case 13:

                if (!buttonslot->getTextData().isEmpty())
                {
                    existingCode->setValue(buttonslot->getTextData(), JoyButtonSlot::JoyTextEntry);
                    existingCode->setToolTip(buttonslot->getTextData());
                }

            break;

            case 14:

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

            break;

            default:

                existingCode->setValue(buttonslot->getSlotCode(),
                                   buttonslot->getSlotCodeAlias(),
                                   buttonslot->getSlotMode());
            break;

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

    if ((m_button->getSetSelection() > -1) &&
        (m_button->getChangeSetCondition() != JoyButton::SetChangeDisabled))
    {
        int selectIndex = static_cast<int>(m_button->getChangeSetCondition());
        selectIndex += m_button->getSetSelection() * 3;
        if (m_button->getOriginSet() < m_button->getSetSelection())
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

    if (m_button->isPartRealAxis() && m_button->isUsingTurbo())
    {
        ui->turboModeComboBox->setEnabled(true);
    }
    else if (!m_button->isPartRealAxis())
    {
        ui->turboModeComboBox->setVisible(false);
        ui->turboModeLabel->setVisible(false);
    }

    findTurboModeComboIndex();

    // Don't show Set Selector page for modifier buttons
    if (m_button->isModifierButton())
    {
        delete ui->listWidget->item(3);
    }

    //performStatsWidgetRefresh(ui->slotListWidget->currentItem());
    changeSlotHelpText(ui->slotTypeComboBox->currentIndex());

    PadderCommon::inputDaemonMutex.unlock();

    ui->resetCycleDoubleSpinBox->setMaximum(GlobalVariables::JoyButton::MAXCYCLERESETTIME * 0.001); // static_cast<double>

    connect(ui->turboCheckbox, &QCheckBox::clicked, ui->turboSlider, &QSlider::setEnabled);
    connect(ui->turboSlider, &QSlider::valueChanged, this, &AdvanceButtonDialog::checkTurboIntervalValue);

    connect(ui->insertSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::insertSlot);
    connect(ui->deleteSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::deleteSlot);
    connect(ui->clearAllPushButton, &QPushButton::clicked, this, &AdvanceButtonDialog::clearAllSlots);

    connect(ui->slotTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::changeSlotTypeDisplay);
    connect(ui->slotTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::changeSlotHelpText);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::updateActionTimeLabel);

    connect(ui->toggleCheckbox, &QCheckBox::clicked, button, &JoyButton::setToggle);
    connect(ui->turboCheckbox, &QCheckBox::clicked, this, &AdvanceButtonDialog::checkTurboSetting);

    connect(ui->setSelectionComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::updateSetSelection);

    connect(ui->slotListWidget, &SlotItemListWidget::itemClicked, this, &AdvanceButtonDialog::performStatsWidgetRefresh);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::checkSlotSetChangeUpdate);

    connect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AdvanceButtonDialog::checkSlotDistanceUpdate);
    connect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AdvanceButtonDialog::checkSlotMouseModUpdate);

    connect(ui->autoResetCycleCheckBox, &QCheckBox::clicked, this, &AdvanceButtonDialog::checkCycleResetWidgetStatus);
    connect(ui->autoResetCycleCheckBox, &QCheckBox::clicked, this, &AdvanceButtonDialog::setButtonCycleReset);
    connect(ui->resetCycleDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &AdvanceButtonDialog::setButtonCycleResetInterval);

    connect(button, &JoyButton::toggleChanged, ui->toggleCheckbox, &QCheckBox::setChecked);
    connect(button, &JoyButton::turboChanged, this, &AdvanceButtonDialog::checkTurboSetting);
    connect(ui->turboModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AdvanceButtonDialog::setButtonTurboMode);
    connect(ui->loadProfilePushButton, &QPushButton::clicked, this, &AdvanceButtonDialog::showSelectProfileWindow);
    connect(ui->execToolButton, &QToolButton::clicked, this, &AdvanceButtonDialog::showFindExecutableWindow);
}

AdvanceButtonDialog::~AdvanceButtonDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void AdvanceButtonDialog::changeTurboText(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        double delay = value / 100.0;
        double clicks = 100.0 / static_cast<double>(value);
        QString delaytext = QString::number(delay, 'g', 3).append(" ").append(trUtf8("sec."));
        QString labeltext = QString::number(clicks, 'g', 2).append(" ").append(trUtf8("/sec."));

        ui->delayValueLabel->setText(delaytext);
        ui->rateValueLabel->setText(labeltext);
    }
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(button, &SimpleKeyGrabberButton::clicked, [this, button]() {

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
    });

    connect(button, &SimpleKeyGrabberButton::buttonCodeChanged, [this, button](int value) {

    JoyButtonSlot *tempbuttonslot = button->getValue();
    int index = ui->slotListWidget->currentRow();

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, tempbuttonslot->getSlotCode()),
                                  Q_ARG(int, tempbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

    updateSlotsScrollArea(value);

    });
}

void AdvanceButtonDialog::deleteSlot()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->slotListWidget->currentRow();
    int itemcount = ui->slotListWidget->count();

    QListWidgetItem *item = ui->slotListWidget->takeItem(index);
    delete item;
    item = nullptr;

    // Deleted last button. Replace with new blank button
    if (index == (itemcount - 1))
    {
        appendBlankKeyGrabber();
    }

    changeTurboForSequences();

    QTimer::singleShot(0, &helper, [this, index]() {
        (&helper)->removeAssignedSlot(index);
    });

    index = qMax(0, index-1);
    performStatsWidgetRefresh(ui->slotListWidget->item(index));

    emit slotsChanged();
}


void AdvanceButtonDialog::appendBlankKeyGrabber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(KBMouseSlot));
}

void AdvanceButtonDialog::insertSlot()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int current = ui->slotListWidget->currentRow();
    int count = ui->slotListWidget->count();
    int slotTypeIndex = ui->slotTypeComboBox->currentIndex();


    switch(slotTypeIndex)
    {
        case 0:

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

        break;

    case 1:
        insertCycleSlot();
        break;

    case 2:
        insertDelaySlot();
        break;

    case 3:
        insertDistanceSlot();
        break;

    case 4:
        insertExecuteSlot();
        break;

    case 5:
        insertHoldSlot();
        break;

    case 6:
        showSelectProfileWindow();
        break;

    case 7:
        insertMouseSpeedModSlot();
        break;

    case 8:
        insertPauseSlot();
        break;

    case 9:
        insertKeyPressSlot();
        break;

    case 10:
        insertReleaseSlot();
        break;

    case 11:
        insertSetChangeSlot();
        break;

    case 12:
        insertTextEntrySlot();
        break;

    }
}

void AdvanceButtonDialog::insertPauseSlot()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->slotListWidget->clear();
    appendBlankKeyGrabber();
    changeTurboForSequences();

    QMetaObject::invokeMethod(m_button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
    performStatsWidgetRefresh(ui->slotListWidget->currentItem());

    emit slotsChanged();
}

void AdvanceButtonDialog::changeTurboForSequences()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            m_button->setUseTurbo(false);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

    qInstallMessageHandler(MessageHandler::myMessageOutput);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        m_button->setTurboInterval(value * 10);
    }
}

void AdvanceButtonDialog::checkTurboSetting(bool state)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->turboCheckbox->setChecked(state);
    ui->turboSlider->setEnabled(state);

    if (m_button->isPartRealAxis())
    {
        ui->turboModeComboBox->setEnabled(state);
    }

    changeTurboForSequences();
    m_button->setUseTurbo(state);
    if ((m_button->getTurboInterval() / 10) >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        ui->turboSlider->setValue(m_button->getTurboInterval() / 10);
    }
}

void AdvanceButtonDialog::updateSetSelection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::inputDaemonMutex.lock();

    int chosen_set = -1;
    JoyButton::SetChangeCondition set_selection_condition = JoyButton::SetChangeDisabled;

    if (ui->setSelectionComboBox->currentIndex() > 0)
    {
        int condition_choice = 0;
        condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;
        chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;

        // Above removed rows
        if (m_button->getOriginSet() > chosen_set)
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
        }
        // Below removed rows
        else
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() + 2) / 3;
        }

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "CONDITION: " << QString::number(condition_choice) << endl;
        #endif


        switch(condition_choice)
        {
        case 0:
            set_selection_condition = JoyButton::SetChangeOneWay;
            break;

        case 1:
            set_selection_condition = JoyButton::SetChangeTwoWay;
            break;

        case 2:
            set_selection_condition = JoyButton::SetChangeWhileHeld;
            break;

        }

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "CHOSEN SET: " << chosen_set << endl;
        #endif
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
        m_button->setChangeSetCondition(JoyButton::SetChangeDisabled);
        m_button->setChangeSetSelection(chosen_set);
        m_button->setChangeSetCondition(set_selection_condition);
    }
    else
    {
        m_button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void AdvanceButtonDialog::checkTurboIntervalValue(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        changeTurboText(value);
        updateTurboIntervalValue(value);
    }
    else
    {
        ui->turboSlider->setValue(GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO);
    }
}

void AdvanceButtonDialog::fillTimeComboBoxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *slot = tempbutton->getValue();

    if ((slot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (slot->getSlotCode() != 0))
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(KBMouseSlot));
    }
    else if ((slot->getSlotMode() == JoyButtonSlot::JoyMouseButton) ||
        (slot->getSlotMode() == JoyButtonSlot::JoyMouseMovement))
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(KBMouseSlot));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(CycleSlot));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDelay)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(DelaySlot));

        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(DistanceSlot));

        disconnect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                   this, &AdvanceButtonDialog::checkSlotDistanceUpdate);
        ui->distanceSpinBox->setValue(slot->getSlotCode());
        connect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                this, &AdvanceButtonDialog::checkSlotDistanceUpdate);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(HoldSlot));
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(LoadSlot));
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(MouseModSlot));
        disconnect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                   this, &AdvanceButtonDialog::checkSlotMouseModUpdate);
        ui->mouseSpeedModSpinBox->setValue(slot->getSlotCode());
        connect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                this, &AdvanceButtonDialog::checkSlotMouseModUpdate);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyPause)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(PauseSlot));
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyKeyPress)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(PressTimeSlot));
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyRelease)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(ReleaseSlot));
        refreshTimeComboBoxes(slot);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoySetChange)
    {
        disconnect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   this, &AdvanceButtonDialog::checkSlotSetChangeUpdate);

        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(SetChangeSlot));
        int chooseIndex = slot->getSlotCode();

        int foundIndex = ui->slotSetChangeComboBox->findData(QVariant(chooseIndex));
        if (foundIndex >= 0)
        {
            ui->slotSetChangeComboBox->setCurrentIndex(foundIndex);
        }

        connect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &AdvanceButtonDialog::checkSlotSetChangeUpdate);
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyTextEntry)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(TextEntry));
        ui->textEntryLineEdit->setText(slot->getTextData());
    }
    else if (slot->getSlotMode() == JoyButtonSlot::JoyExecute)
    {
        ui->slotTypeComboBox->setCurrentIndex(static_cast<int>(ExecuteSlot));
        ui->execLineEdit->setText(slot->getTextData());
        ui->execArgumentsLineEdit->setText(slot->getExtraData().toString());
    }
}

void AdvanceButtonDialog::checkSlotTimeUpdate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton = ui->slotListWidget->currentItem()
            ->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();

    if (buttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    temp.append(trUtf8("Advanced").append(": ")).append(m_button->getPartialName(false, true));

    if (m_button->getParentSet()->getIndex() != 0)
    {
        int setIndex = m_button->getParentSet()->getRealIndex();
        temp.append(" [").append(trUtf8("Set %1").arg(setIndex));

        QString setName = m_button->getParentSet()->getName();
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int milliseconds = (static_cast<int>(value) * 1000) + (static_cast<int>(fmod(value, 1.0)) * 1000);
    m_button->setCycleResetTime(milliseconds);
}

void AdvanceButtonDialog::populateAutoResetInterval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double seconds = m_button->getCycleResetTime() / 1000.0;
    ui->resetCycleDoubleSpinBox->setValue(seconds);
}

void AdvanceButtonDialog::setButtonCycleReset(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (enabled)
    {
        m_button->setCycleResetStatus(true);
        if ((m_button->getCycleResetTime() == 0) && (ui->resetCycleDoubleSpinBox->value() > 0.0))
        {
            double current = ui->resetCycleDoubleSpinBox->value();
            setButtonCycleResetInterval(current);
        }
    }
    else
    {
        m_button->setCycleResetStatus(false);
    }
}

void AdvanceButtonDialog::resetTimeBoxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::updateActionTimeLabel);

    disconnect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &AdvanceButtonDialog::checkSlotTimeUpdate);
}

void AdvanceButtonDialog::connectTimeBoxesEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::updateActionTimeLabel);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AdvanceButtonDialog::checkSlotTimeUpdate);
}

void AdvanceButtonDialog::populateSetSelectionComboBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->setSelectionComboBox->clear();
    ui->setSelectionComboBox->insertItem(0, trUtf8("Disabled"));

    int currentIndex = 1;
    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        if (m_button->getOriginSet() != i)
        {
            QString temp = QString();
            temp.append(trUtf8("Select Set %1").arg(i+1));

            InputDevice *tempdevice = m_button->getParentSet()->getInputDevice();
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->slotSetChangeComboBox->clear();

    int currentIndex = 0;
    for (int i=0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        if (m_button->getOriginSet() != i)
        {
            QString temp = QString();
            temp.append(trUtf8("Select Set %1").arg(i+1));

            InputDevice *tempdevice = m_button->getParentSet()->getInputDevice();
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::TurboMode currentTurboMode = m_button->getTurboMode();

    switch(static_cast<int>(currentTurboMode))
    {

        case 0:
            ui->turboModeComboBox->setCurrentIndex(0);
            break;

        case 1:
            ui->turboModeComboBox->setCurrentIndex(1);
            break;

        case 2:
            ui->turboModeComboBox->setCurrentIndex(2);
            break;

    }
}

void AdvanceButtonDialog::setButtonTurboMode(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    switch(value)
    {
        case 0:
            m_button->setTurboMode(JoyButton::NormalTurbo);
            break;

        case 1:
            m_button->setTurboMode(JoyButton::GradientTurbo);
            break;

        case 2:
            m_button->setTurboMode(JoyButton::PulseTurbo);
            break;
    }
}

void AdvanceButtonDialog::showSelectProfileWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    AntiMicroSettings *settings = m_button->getParentSet()->getInputDevice()->getSettings();

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    switch(index)
    {
        case 0:
        case 2:
        case 5:
        case 8:
        case 9:
        case 10:
            ui->slotControlsStackedWidget->setCurrentIndex(0);
        break;

        case 1:
            ui->slotControlsStackedWidget->setCurrentIndex(3);
        break;

        case 3:
            ui->slotControlsStackedWidget->setCurrentIndex(2);
        break;

        case 4:
            ui->slotControlsStackedWidget->setCurrentIndex(7);
        break;

        case 6:
            ui->slotControlsStackedWidget->setCurrentIndex(4);
        break;

        case 7:
            ui->slotControlsStackedWidget->setCurrentIndex(1);
        break;

        case 11:
            ui->slotControlsStackedWidget->setCurrentIndex(5);
        break;

        case 12:
            ui->slotControlsStackedWidget->setCurrentIndex(6);
        break;
    }
}

void AdvanceButtonDialog::changeSlotHelpText(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    switch(index)
    {
        case 0:
            ui->slotTypeHelpLabel->setText(trUtf8("Insert a new blank slot."));
        break;

        case 1:
            ui->slotTypeHelpLabel->setText(trUtf8("Slots past a Cycle action will be executed "
                                          "on the next button press. Multiple cycles can be added "
                                          "in order to create partitions in a sequence."));
        break;

        case 2:
            ui->slotTypeHelpLabel->setText(trUtf8("Delays the time that the next slot is activated "
                                          "by the time specified. Slots activated before the "
                                          "delay will remain active after the delay time "
                                          "has passed."));
        break;

        case 3:
            ui->slotTypeHelpLabel->setText(trUtf8("Distance action specifies that the slots afterwards "
                                          "will only be executed when an axis is moved "
                                          "a certain range past the designated dead zone."));
        break;

        case 4:
            ui->slotTypeHelpLabel->setText(trUtf8("Execute program when slot is activated."));
        break;

        case 5:
            ui->slotTypeHelpLabel->setText(trUtf8("Insert a hold action. Slots after the action will only be "
                                          "executed if the button is held past the interval specified."));
        break;

        case 6:
            ui->slotTypeHelpLabel->setText(trUtf8("Chose a profile to load when this slot is activated."));
        break;

        case 7:
            ui->slotTypeHelpLabel->setText(trUtf8("Mouse mod action will modify all mouse speed settings "
                                          "by a specified percentage while the action is being processed. "
                                          "This can be useful for slowing down the mouse while "
                                          "sniping."));
        break;

        case 8:
            ui->slotTypeHelpLabel->setText(trUtf8("Insert a pause that occurs in between key presses."));
        break;

        case 9:
            ui->slotTypeHelpLabel->setText(trUtf8("Specify the time that keys past this slot should be "
                                          "held down."));
        break;

        case 10:
            ui->slotTypeHelpLabel->setText(trUtf8("Insert a release action. Slots after the action will only be "
                                          "executed after a button release if the button was held "
                                          "past the interval specified."));
        break;

        case 11:
            ui->slotTypeHelpLabel->setText(trUtf8("Change to selected set once slot is activated."));
        break;

        case 12:
            ui->slotTypeHelpLabel->setText(trUtf8("Full string will be typed when a "
                                          "slot is activated."));
        break;

    }
}

int AdvanceButtonDialog::getOldRow() const {

    return oldRow;
}

JoyButton *AdvanceButtonDialog::getButton() const {

    return m_button;
}

AdvanceButtonDialogHelper const& AdvanceButtonDialog::getHelper() {

    return helper;
}

AdvanceButtonDialogHelper& AdvanceButtonDialog::getHelperLocal() {

    return helper;
}
