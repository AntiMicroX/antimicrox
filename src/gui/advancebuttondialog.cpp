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

#include "advancebuttondialog.h"
#include "ui_advancebuttondialog.h"

#include "event.h"
#include "globalvariables.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"
#include "joytabwidget.h"
#include "simplekeygrabberbutton.h"
//#include "logger.h"

#include <cmath>

#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QtGlobal>
//#include <QTest>

AdvanceButtonDialog::AdvanceButtonDialog(JoyButton *button, QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AdvanceButtonDialog)
    , helper(button)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->page);
    setAttribute(Qt::WA_DeleteOnClose);

    PadderCommon::inputDaemonMutex.lock();

    // ui->splitSlotButton->hide();

    m_button = button;
    oldRow = 0;
    int interval = m_button->getTurboInterval() / 10;

    getHelperLocal().moveToThread(button->thread());

    if (m_button->getToggleState())
        ui->toggleCheckbox->setChecked(true);

    if (m_button->isUsingTurbo())
    {
        ui->turboCheckbox->setChecked(true);
        ui->turboSlider->setEnabled(true);
    }

    if (interval < GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
        interval = GlobalVariables::JoyButton::ENABLEDTURBODEFAULT / 10;

    ui->turboSlider->setValue(interval);
    this->changeTurboText(interval);

    QListIterator<JoyButtonSlot *> iter(*(m_button->getAssignedSlots()));

    while (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        qDebug() << "slot string for grab btn is: " << buttonslot->getSlotString();
        SimpleKeyGrabberButton *existingCode = new SimpleKeyGrabberButton(this);
        existingCode->setText(buttonslot->getSlotString());

        switch (static_cast<int>(buttonslot->getSlotMode()))
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
                    existingCode->getValue()->setExtraData(buttonslot->getExtraData().toString());
            }

            break;

        // JoyMix
        case 15:

            qDebug() << "text data for joy mix is: " << buttonslot->getTextData();
            qDebug() << "slot string for joy mix is: " << buttonslot->getSlotString();
            qDebug() << "amount of mini slots: " << buttonslot->getMixSlots()->count();

            if (!buttonslot->getTextData().isEmpty())
            {
                existingCode->setValues(buttonslot->getTextData(), buttonslot->getMixSlots(),
                                        JoyButtonSlot::JoySlotInputAction::JoyMix);
                existingCode->setToolTip(buttonslot->getTextData());

                // try again
                if (existingCode->text() == tr("[NO KEY]"))
                {
                    existingCode->setValues(buttonslot->getTextData(), buttonslot->getMixSlots(),
                                            JoyButtonSlot::JoySlotInputAction::JoyMix);
                    existingCode->setToolTip(buttonslot->getTextData());
                    existingCode->setText(buttonslot->getSlotString());
                }

                qDebug() << "Existing code for Joy Mix: " << existingCode->text();
            }

            break;

        default:

            existingCode->setValue(buttonslot->getSlotCode(), buttonslot->getSlotCodeAlias(), buttonslot->getSlotMode());
            break;
        }

        QListWidgetItem *item = new QListWidgetItem(ui->slotListWidget);
        item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton *>(existingCode));
        QHBoxLayout *layout = new QHBoxLayout();
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

    if ((m_button->getSetSelection() > -1) && (m_button->getChangeSetCondition() != JoyButton::SetChangeDisabled))
    {
        int selectIndex = m_button->getChangeSetCondition();
        selectIndex += m_button->getSetSelection() * 3;

        if (m_button->getOriginSet() < m_button->getSetSelection())
            selectIndex -= 3;

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
        populateAutoResetInterval();

    updateWindowTitleButtonName();

    if (m_button->isPartRealAxis() && m_button->isUsingTurbo())
    {
        ui->turboModeComboBox->setEnabled(true);
    } else if (!m_button->isPartRealAxis())
    {
        ui->turboModeComboBox->setVisible(false);
        ui->turboModeLabel->setVisible(false);
    }

    findTurboModeComboIndex();

    // Don't show Set Selector page for modifier buttons
    if (m_button->isModifierButton())
        delete ui->listWidget->item(3);

    changeSlotHelpText(ui->slotTypeComboBox->currentIndex());

    PadderCommon::inputDaemonMutex.unlock();

    ui->resetCycleDoubleSpinBox->setMaximum(GlobalVariables::JoyButton::MAXCYCLERESETTIME * 0.001); // static_cast<double>

    connect(ui->turboCheckbox, &QCheckBox::clicked, ui->turboSlider, &QSlider::setEnabled);
    connect(ui->turboSlider, &QSlider::valueChanged, this, &AdvanceButtonDialog::checkTurboIntervalValue);

    connect(ui->insertSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::insertSlot);
    connect(ui->joinSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::joinSlot);
    connect(ui->splitSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::splitSlot);
    connect(ui->deleteSlotButton, &QPushButton::clicked, this, &AdvanceButtonDialog::deleteSlot);
    connect(ui->clearAllPushButton, &QPushButton::clicked, this, &AdvanceButtonDialog::clearAllSlots);

    connect(ui->slotTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::changeSlotTypeDisplay);
    connect(ui->slotTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::changeSlotHelpText);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);

    connect(ui->toggleCheckbox, &QCheckBox::clicked, button, &JoyButton::setToggle);
    connect(ui->turboCheckbox, &QCheckBox::clicked, this, &AdvanceButtonDialog::checkTurboSetting);

    connect(ui->setSelectionComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateSetSelection);

    connect(ui->slotListWidget, &SlotItemListWidget::itemClicked, this, &AdvanceButtonDialog::performStatsWidgetRefresh);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotSetChangeUpdate);

    connect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &AdvanceButtonDialog::checkSlotDistanceUpdate);
    connect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &AdvanceButtonDialog::checkSlotMouseModUpdate);

    connect(ui->autoResetCycleCheckBox, &QCheckBox::clicked, this, &AdvanceButtonDialog::checkCycleResetWidgetStatus);
    connect(ui->autoResetCycleCheckBox, &QCheckBox::clicked, this, &AdvanceButtonDialog::setButtonCycleReset);
    connect(ui->resetCycleDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &AdvanceButtonDialog::setButtonCycleResetInterval);

    connect(button, &JoyButton::toggleChanged, ui->toggleCheckbox, &QCheckBox::setChecked);
    connect(button, &JoyButton::turboChanged, this, &AdvanceButtonDialog::checkTurboSetting);

    connect(ui->turboModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::setButtonTurboMode);
    connect(ui->loadProfilePushButton, &QPushButton::clicked, this, &AdvanceButtonDialog::showSelectProfileWindow);
    connect(ui->execToolButton, &QToolButton::clicked, this, &AdvanceButtonDialog::showFindExecutableWindow);
}

// for tests
AdvanceButtonDialog::AdvanceButtonDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AdvanceButtonDialog)
    , helper(nullptr)
{
}

AdvanceButtonDialog::~AdvanceButtonDialog() { delete ui; }

void AdvanceButtonDialog::changeTurboText(int value)
{
    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        double delay = value / 100.0;
        double clicks = 100.0 / value;
        QString delaytext = QString::number(delay, 'g', 3).append(" ").append(tr("sec."));
        QString labeltext = QString::number(clicks, 'g', 2).append(" ").append(tr("/sec."));

        ui->delayValueLabel->setText(delaytext);
        ui->rateValueLabel->setText(labeltext);
    }
}

void AdvanceButtonDialog::updateSlotsScrollArea(int value)
{
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
    connect(button, &SimpleKeyGrabberButton::clicked, [this, button]() {
        bool leave = false;

        for (int i = 0; (i < ui->slotListWidget->count()) && !leave; i++)
        {
            QListWidgetItem *item = ui->slotListWidget->item(i);
            SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();

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

        if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoySlotInputAction::JoyMix)
        {
            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(JoyButtonSlot *, tempbuttonslot), Q_ARG(int, index));
        } else
        {
            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, tempbuttonslot->getSlotCode()),
                                      Q_ARG(int, tempbuttonslot->getSlotCodeAlias()), Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

            updateSlotsScrollArea(value);
        }
    });
}

void AdvanceButtonDialog::deleteSlot(bool showWarning)
{
    if (ui->slotListWidget->selectedItems().count() == 0)
    {
        if (showWarning)
            QMessageBox::warning(this, tr("Not checked slots"), tr("To delete slots, you need to select at least one"));
    } else
    {
        for (auto item : ui->slotListWidget->selectedItems())
        {
            int index = ui->slotListWidget->row(item);
            int itemcount = ui->slotListWidget->count();
            delete ui->slotListWidget->takeItem(index);

            // Deleted last button. Replace with new blank button
            if (index == (itemcount - 1))
                appendBlankKeyGrabber();

            changeTurboForSequences();

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
            QTimer::singleShot(0, &helper, [this, index]() { (&helper)->removeAssignedSlot(index); });
#else
            QMetaObject::invokeMethod(&helper, "removeAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, index));
#endif

            index = qMax(0, index - 1);
            performStatsWidgetRefresh(ui->slotListWidget->item(index));

            emit slotsChanged();
        }
    }
}

void AdvanceButtonDialog::appendBlankKeyGrabber()
{
    SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
    QListWidgetItem *item = new QListWidgetItem(ui->slotListWidget);
    item->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton *>(blankButton));

    QHBoxLayout *layout = new QHBoxLayout();
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
    if (ui->slotListWidget->selectedItems().count() == 0)
    {
        QMessageBox::warning(this, tr("Not checked slots"), tr("To insert slots, you need to select at least one"));
    } else
    {
        QStringList firstChoiceExec = QStringList();
        QString firstChoiceProfile = QString();

        for (auto item : ui->slotListWidget->selectedItems())
        {
            int current = ui->slotListWidget->row(item);
            int count = ui->slotListWidget->count();
            int slotTypeIndex = ui->slotTypeComboBox->currentIndex();

            switch (slotTypeIndex)
            {
            case 0:

                if (current != (count - 1))
                {
                    SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
                    QListWidgetItem *itemListWidget = new QListWidgetItem();
                    ui->slotListWidget->insertItem(current, itemListWidget);
                    itemListWidget->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton *>(blankButton));

                    QHBoxLayout *layout = new QHBoxLayout();
                    layout->addWidget(blankButton);
                    QWidget *widget = new QWidget();
                    widget->setLayout(layout);
                    itemListWidget->setSizeHint(widget->sizeHint());
                    ui->slotListWidget->setItemWidget(itemListWidget, widget);
                    ui->slotListWidget->setCurrentItem(itemListWidget);
                    connectButtonEvents(blankButton);
                    blankButton->refreshButtonLabel();

                    QMetaObject::invokeMethod(&helper, "insertAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, 0),
                                              Q_ARG(int, 0), Q_ARG(int, current));

                    updateSlotsScrollArea(0);
                }

                break;

            case 1:
                insertCycleSlot(item);
                break;

            case 2:
                insertKindOfSlot(item, actionTimeConvert(), JoyButtonSlot::JoyDelay);
                break;

            case 3:
                insertKindOfSlot(item, ui->distanceSpinBox->value(), JoyButtonSlot::JoyDistance);
                break;

            case 4:
                insertExecuteSlot(item, firstChoiceExec);
                break;

            case 5:
                insertKindOfSlot(item, actionTimeConvert(), JoyButtonSlot::JoyHold);
                break;

            case 6:
                showSelectProfileWind(item, firstChoiceProfile);
                break;

            case 7:
                insertKindOfSlot(item, ui->mouseSpeedModSpinBox->value(), JoyButtonSlot::JoyMouseSpeedMod);
                break;

            case 8:
                insertKindOfSlot(item, actionTimeConvert(), JoyButtonSlot::JoyPause);
                break;

            case 9:
                insertKindOfSlot(item, actionTimeConvert(), JoyButtonSlot::JoyKeyPress);
                break;

            case 10:
                insertKindOfSlot(item, actionTimeConvert(), JoyButtonSlot::JoyRelease);
                break;

            case 11:
                insertKindOfSlot(item,
                                 ui->slotSetChangeComboBox->itemData(ui->slotSetChangeComboBox->currentIndex()).toInt(),
                                 JoyButtonSlot::JoySetChange);
                break;

            case 12:
                insertTextEntrySlot(item);
                break;
            }
        }
    }
}

void AdvanceButtonDialog::joinSlot()
{
    QReadLocker tempAssignLocker(&joinLock);
    joinLock.lockForRead();

    if (ui->slotListWidget->currentRow() == -1)
    {
        QMessageBox::warning(this, tr("Unknown current slot"), tr("Click on chosen slots before joining them"));
    } else if (ui->slotListWidget->count() < 3)
    {
        QMessageBox::warning(this, tr("Not enough slots"),
                             tr("It's impossible to join slots. Add at least one other slot."));
    } else if (ui->slotListWidget->selectedItems().count() < 2)
    {
        QMessageBox::warning(this, tr("Not selected slots"),
                             tr("It's impossible to join slots. Select at least two slots before joining them"));
    } else if (anySelectedNotKeybSlot())
    {
        QMessageBox::warning(this, tr("Only keyboard slots"),
                             tr("It's only possible to join simple and mix keyboard slots"));
    } else
    {
        qDebug() << "Chosen " << ui->slotListWidget->selectedItems().count() << " slots";

        QListWidgetItem *firstSelected = ui->slotListWidget->selectedItems().at(0);
        QString text = "";
        int index = ui->slotListWidget->row(firstSelected);
        bool firstTime = true;

        SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);
        QList<QListWidgetItem *> listItems = ui->slotListWidget->selectedItems();

        for (auto item : listItems)
        {
            if (!firstTime)
                text += "+";
            firstTime = false;

            SimpleKeyGrabberButton *firstGrabBtn = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();

            if (firstGrabBtn->getValue()->getMixSlots()->count() > 0)
            {
                QList<JoyButtonSlot *> slotsList;

                for (auto mini : *firstGrabBtn->getValue()->getMixSlots())
                    slotsList.append(new JoyButtonSlot(mini->getSlotCode(), mini->getSlotCodeAlias(), mini->getSlotMode()));

                // Q_ASSERT(blankButton->getValue() != nullptr);
                blankButton->getValue()->appendMiniSlot<QList<JoyButtonSlot *>>(slotsList);
            } else
            {
                JoyButtonSlot *slotmini =
                    new JoyButtonSlot(firstGrabBtn->getValue()->getSlotCode(), firstGrabBtn->getValue()->getSlotCodeAlias(),
                                      firstGrabBtn->getValue()->getSlotMode());

                // Q_ASSERT(blankButton->getValue() != nullptr);
                blankButton->getValue()->appendMiniSlot<JoyButtonSlot *>(slotmini);
            }

            text += firstGrabBtn->getValue()->getSlotString();
        }

        blankButton->getValue()->setTextData(text);
        blankButton->getValue()->setSlotMode(JoyButtonSlot::JoyMix);
        blankButton->getValue()->setSlotCode(-1);

        deleteSlot(false);

        // Q_ASSERT(blankButton->getValue()->getMixSlots() != nullptr);
        // Q_ASSERT(blankButton->getValue()->getMixSlots()->count() > 0);

        for (auto x : *blankButton->getValue()->getMixSlots())
        {
            //   Q_ASSERT(x->getSlotMode() == 0);
            //   Q_ASSERT(!x->getSlotString().isEmpty());
            qDebug() << "JOINED MINI: " << x->getSlotCode() << " - " << x->getSlotMode() << " - " << x->getSlotString();
        }

        QListWidgetItem *joinedItem = new QListWidgetItem();
        ui->slotListWidget->insertItem(qMax(0, index), joinedItem);

        joinedItem->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton *>(blankButton));

        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(blankButton);
        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        joinedItem->setSizeHint(widget->sizeHint());
        ui->slotListWidget->setItemWidget(joinedItem, widget);
        ui->slotListWidget->setCurrentItem(joinedItem);

        // blankButton->setValues(text, blankButton->getValue()->getMixSlots(), JoyButtonSlot::JoyMix);

        connectButtonEvents(blankButton);
        blankButton->refreshButtonLabel(); // instead of blankButton->setText(text);

        QMetaObject::invokeMethod(&helper, "insertAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot *, blankButton->getValue()), Q_ARG(int, index), Q_ARG(bool, false));
    }

    joinLock.unlock();
}

void AdvanceButtonDialog::splitSlot()
{
    int index = ui->slotListWidget->currentRow();

    if (index == -1)
    {
        QMessageBox::warning(this, tr("Unknown current slot"), tr("Click on chosen slots before joining them"));
    } else if (ui->slotListWidget->count() < 2)
    {
        QMessageBox::warning(this, tr("Not enough slots"),
                             tr("It's impossible to split slots. Add at least one other slot."));
    } else if (ui->slotListWidget->selectedItems().count() < 1)
    {
        QMessageBox::warning(this, tr("Not selected slot"), tr("Select your slot before splitting."));
    } else if (ui->slotListWidget->selectedItems().count() > 1)
    {
        QMessageBox::warning(this, tr("Too many mix slots"), tr("Select one mix slot."));
    } else if (selectedNotMixSlot())
    {
        QMessageBox::warning(this, tr("Only mix slots"), tr("It's only possible to split mix slot."));
    } else
    {
        QReadLocker tempAssignLocker(&joinLock);
        joinLock.lockForRead();

        QListWidgetItem *mixSlot = ui->slotListWidget->selectedItems().at(0);
        int indexMixSlot = ui->slotListWidget->row(mixSlot);
        int indexKeyboardSlot = indexMixSlot;

        QList<JoyButtonSlot *> minislots =
            *mixSlot->data(Qt::UserRole).value<SimpleKeyGrabberButton *>()->getValue()->getMixSlots();

        for (auto minislot : minislots)
        {
            qDebug() << "MINISLOT SPLIT NAME: " << minislot->getSlotString();
            QListWidgetItem *splitItem = new QListWidgetItem();

            SimpleKeyGrabberButton *blankButton = new SimpleKeyGrabberButton(this);

            JoyButtonSlot *slotmini =
                new JoyButtonSlot(minislot->getSlotCode(), minislot->getSlotCodeAlias(), minislot->getSlotMode());

            ui->slotListWidget->insertItem(qMax(0, indexKeyboardSlot), splitItem);

            blankButton->setValue(slotmini);

            delete slotmini;
            slotmini = nullptr;

            splitItem->setData(Qt::UserRole, QVariant::fromValue<SimpleKeyGrabberButton *>(blankButton));

            QHBoxLayout *layout = new QHBoxLayout();
            layout->addWidget(blankButton);
            QWidget *widget = new QWidget();
            widget->setLayout(layout);
            splitItem->setSizeHint(widget->sizeHint());
            ui->slotListWidget->setItemWidget(splitItem, widget);
            // ui->slotListWidget->setCurrentItem(splitItem);

            connectButtonEvents(blankButton);
            blankButton->refreshButtonLabel(); // instead of blankButton->setText(text);

            QMetaObject::invokeMethod(&helper, "insertAssignedSlot", Qt::BlockingQueuedConnection,
                                      Q_ARG(int, blankButton->getValue()->getSlotCode()),
                                      Q_ARG(int, blankButton->getValue()->getSlotCodeAlias()), Q_ARG(int, indexKeyboardSlot),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, blankButton->getValue()->getSlotMode()));

            indexKeyboardSlot++;
        }

        // it can be used as reusable code
        deleteSlot(false);

        joinLock.unlock();
    }
}

void AdvanceButtonDialog::insertKindOfSlot(QListWidgetItem *item, int slotProperty,
                                           JoyButtonSlot::JoySlotInputAction inputAction)
{
    int index = ui->slotListWidget->row(item);
    int actionTime = slotProperty;
    SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();

    int tempDistance = 0;
    bool slotPropertyAboveLimit = actionTime >= 0;

    if (inputAction == JoyButtonSlot::JoyDistance)
    {
        for (int i = 0; i < ui->slotListWidget->count(); i++)
        {
            SimpleKeyGrabberButton *button =
                ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
            JoyButtonSlot *tempbuttonslot = button->getValue();

            if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
            {
                tempDistance += tempbuttonslot->getSlotCode();
            } else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
            {
                tempDistance = 0;
            }
        }

        tempDistance += actionTime;
        slotPropertyAboveLimit = tempDistance <= 100;
    }

    if (slotPropertyAboveLimit)
    {
        tempbutton->setValue(actionTime, inputAction);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, actionTime),
                                  Q_ARG(int, 0), Q_ARG(int, index), Q_ARG(JoyButtonSlot::JoySlotInputAction, inputAction));

        updateSlotsScrollArea(actionTime);
    }
}

int AdvanceButtonDialog::actionTimeConvert()
{
    int tempMilliSeconds = ui->actionMinutesComboBox->currentIndex() * 1000 * 60;
    tempMilliSeconds += ui->actionSecondsComboBox->currentIndex() * 1000;
    tempMilliSeconds += ui->actionTenthsComboBox->currentIndex() * 100;
    tempMilliSeconds += ui->actionHundredthsComboBox->currentIndex() * 10;

    return tempMilliSeconds;
}

void AdvanceButtonDialog::refreshTimeComboBoxes(JoyButtonSlot *slot)
{
    disconnectTimeBoxesEvents();

    int slottime = slot->getSlotCode();

    ui->actionMinutesComboBox->setCurrentIndex(slottime / 1000 / 60);
    ui->actionSecondsComboBox->setCurrentIndex(slottime / 1000 % 60);
    ui->actionTenthsComboBox->setCurrentIndex((slottime % 1000) / 100);
    ui->actionHundredthsComboBox->setCurrentIndex((slottime % 1000 % 100) / 10);
    updateActionTimeLabel();

    connectTimeBoxesEvents();
}

void AdvanceButtonDialog::updateActionTimeLabel()
{
    int actionTime = actionTimeConvert();
    int minutes = actionTime / 1000 / 60;
    double hundredths = actionTime % 1000 / 1000.0;
    double seconds = (actionTime / 1000 % 60) + hundredths;
    QString actionTimeLabel = QString();

    actionTimeLabel.append(QString::number(minutes)).append("m ");
    actionTimeLabel.append(QString::number(seconds, 'f', 2)).append("s");
    ui->actionTimeLabel->setText(actionTimeLabel);
}

void AdvanceButtonDialog::clearAllSlots()
{
    ui->slotListWidget->clear();
    appendBlankKeyGrabber();
    changeTurboForSequences();

    QMetaObject::invokeMethod(m_button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
    performStatsWidgetRefresh(ui->slotListWidget->currentItem());

    emit slotsChanged();
}

void AdvanceButtonDialog::changeTurboForSequences()
{
    bool containsSequences = false;

    for (int i = 0; (i < ui->slotListWidget->count()) && !containsSequences; i++)
    {
        SimpleKeyGrabberButton *button = ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
        JoyButtonSlot *tempbuttonslot = button->getValue();

        if ((tempbuttonslot->getSlotCode() > 0) && ((tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyPause) ||
                                                    (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyHold) ||
                                                    (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)))
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
    } else if (!ui->turboCheckbox->isEnabled())
    {
        ui->turboCheckbox->setEnabled(true);
        emit turboButtonEnabledChange(true);
    }
}

void AdvanceButtonDialog::insertCycleSlot(QListWidgetItem *item)
{
    int index = ui->slotListWidget->row(item);
    SimpleKeyGrabberButton *tempbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    tempbutton->setValue(1, JoyButtonSlot::JoyCycle);

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, 1), Q_ARG(int, 0),
                              Q_ARG(int, index), Q_ARG(JoyButtonSlot::JoySlotInputAction, JoyButtonSlot::JoyCycle));

    updateSlotsScrollArea(1);
}

void AdvanceButtonDialog::placeNewSlot(JoyButtonSlot *slot)
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton =
        ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    tempbutton->setValue(slot->getSlotCode(), slot->getSlotCodeAlias(), slot->getSlotMode());

    QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, slot->getSlotCode()),
                              Q_ARG(int, slot->getSlotCodeAlias()), Q_ARG(int, index),
                              Q_ARG(JoyButtonSlot::JoySlotInputAction, slot->getSlotMode()));

    updateSlotsScrollArea(slot->getSlotCode());

    slot->deleteLater();
}

void AdvanceButtonDialog::updateTurboIntervalValue(int value)
{
    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        m_button->setTurboInterval(value * 10);
    }
}

void AdvanceButtonDialog::checkTurboSetting(bool state)
{
    ui->turboCheckbox->setChecked(state);
    ui->turboSlider->setEnabled(state);

    if (m_button->isPartRealAxis())
        ui->turboModeComboBox->setEnabled(state);

    changeTurboForSequences();
    m_button->setUseTurbo(state);

    if ((m_button->getTurboInterval() / 10) >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        ui->turboSlider->setValue(m_button->getTurboInterval() / 10);
    }
}

void AdvanceButtonDialog::updateSetSelection()
{
    PadderCommon::inputDaemonMutex.lock();

    int chosen_set;
    JoyButton::SetChangeCondition set_selection_condition = JoyButton::SetChangeDisabled;

    if (ui->setSelectionComboBox->currentIndex() > 0)
    {
        int condition_choice = (ui->setSelectionComboBox->currentIndex() + 2) % 3;
        chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;

        if (m_button->getOriginSet() > chosen_set) // Above removed rows
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() - 1) / 3;
        } else // Below removed rows
        {
            chosen_set = (ui->setSelectionComboBox->currentIndex() + 2) / 3;
        }

        qDebug() << "CONDITION: " << QString::number(condition_choice);

        switch (condition_choice)
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

        qDebug() << "CHOSEN SET: " << chosen_set;

    } else
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
    } else
    {
        m_button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void AdvanceButtonDialog::checkTurboIntervalValue(int value)
{
    if (value >= GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO)
    {
        changeTurboText(value);
        updateTurboIntervalValue(value);
    } else
    {
        ui->turboSlider->setValue(GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO);
    }
}

void AdvanceButtonDialog::fillTimeComboBoxes()
{
    ui->actionMinutesComboBox->clear();
    ui->actionSecondsComboBox->clear();
    ui->actionHundredthsComboBox->clear();
    ui->actionTenthsComboBox->clear();

    for (int i = 0; i <= 10; i++)
    {
        QString minText = QString::number(i, 'g', 2).append("m");
        ui->actionMinutesComboBox->addItem(minText);
    }

    for (int i = 0; i <= 59; i++)
    {
        QString secsText = QString::number(i, 'g', 2);
        ui->actionSecondsComboBox->addItem(secsText);
    }

    for (int i = 0; i < 10; i++)
    {
        QString tenthsText = QString(".%1").arg(i, 1, 10, QChar('0'));
        ui->actionTenthsComboBox->addItem(tenthsText);
    }

    for (int i = 0; i < 10; i++)
    {
        QString hundrText = QString("%1s").arg(i, 1, 10, QChar('0'));
        ui->actionHundredthsComboBox->addItem(hundrText);
    }
}

void AdvanceButtonDialog::insertTextEntrySlot(QListWidgetItem *item)
{
    int index = ui->slotListWidget->row(item);
    QString entryText = ui->textEntryLineEdit->text();
    SimpleKeyGrabberButton *textEntryButton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();

    if (!entryText.isEmpty())
    {
        textEntryButton->setValue(entryText, JoyButtonSlot::JoyTextEntry);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot *, textEntryButton->getValue()), Q_ARG(int, index));

        textEntryButton->setToolTip(entryText);
        updateSlotsScrollArea(0);
    }
}

void AdvanceButtonDialog::insertExecuteSlot(QListWidgetItem *item, QStringList &prevExecAndArgs)
{
    int index = ui->slotListWidget->row(item);
    QString execSlotName, argsExecSlot;
    QFile execFile;
    QFileInfo execSlotNameInfo;

    if (prevExecAndArgs.empty()) // the first time when we choose script
    {
        execSlotName = ui->execLineEdit->text();
        argsExecSlot = ui->execArgumentsLineEdit->text();
    } else // when we want to apply changes to many slots at once
    {
        execSlotName = prevExecAndArgs.first();

        if (prevExecAndArgs.length() == 2)
            argsExecSlot = prevExecAndArgs.last();
    }

    execFile.setFileName(execSlotName);
    execSlotNameInfo.setFile(execSlotName);

    SimpleKeyGrabberButton *execbutton = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();

    if (execSlotName.isEmpty())
        QMessageBox::warning(
            this, tr("Empty execution path"),
            tr("Line for execution file path is empty. Fill the first line before you are going to add a slot."));
    else if (!execSlotNameInfo.exists())
        QMessageBox::warning(this, tr("File doesn't exist"),
                             tr("There is no such file locally, that could be executed. Check the file on your system"));
    else
    {
        prevExecAndArgs.clear();
        execbutton->setValue(execSlotName, JoyButtonSlot::JoyExecute);
        prevExecAndArgs << execSlotName;

        if (!argsExecSlot.isEmpty())
        {
            execbutton->getValue()->setExtraData(QVariant(argsExecSlot));
            prevExecAndArgs << argsExecSlot;
        }

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot *, execbutton->getValue()), Q_ARG(int, index));

        execbutton->setToolTip(execSlotName);
        updateSlotsScrollArea(0);
    }
}

void AdvanceButtonDialog::performStatsWidgetRefresh(QListWidgetItem *item)
{
    SimpleKeyGrabberButton *button = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    JoyButtonSlot *slot = button->getValue();

    if ((slot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (slot->getSlotCode() != 0))
    {
        ui->slotTypeComboBox->setCurrentIndex(0);
    } else if ((slot->getSlotMode() == JoyButtonSlot::JoyMouseButton) ||
               (slot->getSlotMode() == JoyButtonSlot::JoyMouseMovement))
    {
        ui->slotTypeComboBox->setCurrentIndex(0);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
    {
        ui->slotTypeComboBox->setCurrentIndex(1);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyDelay)
    {
        ui->slotTypeComboBox->setCurrentIndex(2);

        refreshTimeComboBoxes(slot);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        ui->slotTypeComboBox->setCurrentIndex(3);

        disconnect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
                   &AdvanceButtonDialog::checkSlotDistanceUpdate);

        ui->distanceSpinBox->setValue(slot->getSlotCode());

        connect(ui->distanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
                &AdvanceButtonDialog::checkSlotDistanceUpdate);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        ui->slotTypeComboBox->setCurrentIndex(5);
        refreshTimeComboBoxes(slot);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        ui->slotTypeComboBox->setCurrentIndex(6);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod)
    {
        ui->slotTypeComboBox->setCurrentIndex(7);
        disconnect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
                   &AdvanceButtonDialog::checkSlotMouseModUpdate);

        ui->mouseSpeedModSpinBox->setValue(slot->getSlotCode());

        connect(ui->mouseSpeedModSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
                &AdvanceButtonDialog::checkSlotMouseModUpdate);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyPause)
    {
        ui->slotTypeComboBox->setCurrentIndex(8);
        refreshTimeComboBoxes(slot);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyKeyPress)
    {
        ui->slotTypeComboBox->setCurrentIndex(9);
        refreshTimeComboBoxes(slot);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyRelease)
    {
        ui->slotTypeComboBox->setCurrentIndex(10);
        refreshTimeComboBoxes(slot);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoySetChange)
    {
        disconnect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                   &AdvanceButtonDialog::checkSlotSetChangeUpdate);

        ui->slotTypeComboBox->setCurrentIndex(11);
        int chooseIndex = slot->getSlotCode();
        int foundIndex = ui->slotSetChangeComboBox->findData(QVariant(chooseIndex));

        if (foundIndex >= 0)
            ui->slotSetChangeComboBox->setCurrentIndex(foundIndex);

        connect(ui->slotSetChangeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &AdvanceButtonDialog::checkSlotSetChangeUpdate);
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyTextEntry)
    {
        ui->slotTypeComboBox->setCurrentIndex(12);
        ui->textEntryLineEdit->setText(slot->getTextData());
    } else if (slot->getSlotMode() == JoyButtonSlot::JoyExecute)
    {
        ui->slotTypeComboBox->setCurrentIndex(4);
        ui->execLineEdit->setText(slot->getTextData());
        ui->execArgumentsLineEdit->setText(slot->getExtraData().toString());
    }
}

void AdvanceButtonDialog::checkSlotTimeUpdate()
{
    int actionTime = actionTimeConvert();
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *button = ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    JoyButtonSlot *buttonSlot = button->getValue();

    if ((buttonSlot->getSlotMode() == JoyButtonSlot::JoyPause) || (buttonSlot->getSlotMode() == JoyButtonSlot::JoyHold) ||
        (buttonSlot->getSlotMode() == JoyButtonSlot::JoyRelease) ||
        (buttonSlot->getSlotMode() == JoyButtonSlot::JoyKeyPress) || (buttonSlot->getSlotMode() == JoyButtonSlot::JoyDelay))
    {
        if (actionTime > 0)
        {
            button->setValue(actionTime, buttonSlot->getSlotMode());

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, actionTime),
                                      Q_ARG(int, 0), Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, buttonSlot->getSlotMode()));

            updateSlotsScrollArea(actionTime);
        }
    }
}

void AdvanceButtonDialog::checkSlotMouseModUpdate()
{
    int tempMouseMod = ui->mouseSpeedModSpinBox->value();
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton =
        ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    JoyButtonSlot *tempbuttonslot = tempbutton->getValue();

    if ((tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyMouseSpeedMod) && (tempMouseMod > 0))
    {
        tempbutton->setValue(tempMouseMod, tempbuttonslot->getSlotMode());

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, tempMouseMod),
                                  Q_ARG(int, 0), Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, tempbuttonslot->getSlotMode()));

        updateSlotsScrollArea(tempMouseMod);
    }
}

void AdvanceButtonDialog::checkSlotSetChangeUpdate()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton =
        ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();

    if (buttonslot->getSlotMode() == JoyButtonSlot::JoySetChange &&
        (ui->slotSetChangeComboBox->itemData(ui->slotSetChangeComboBox->currentIndex()).toInt() >= 0))
    {
        int comboIndex = ui->slotSetChangeComboBox->currentIndex();
        int setIndex = ui->slotSetChangeComboBox->itemData(comboIndex).toInt();
        tempbutton->setValue(setIndex, buttonslot->getSlotMode());

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, setIndex),
                                  Q_ARG(int, 0), Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, buttonslot->getSlotMode()));

        updateSlotsScrollArea(setIndex);
    }
}

void AdvanceButtonDialog::checkSlotDistanceUpdate()
{
    int index = ui->slotListWidget->currentRow();
    SimpleKeyGrabberButton *tempbutton =
        ui->slotListWidget->currentItem()->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
    JoyButtonSlot *buttonslot = tempbutton->getValue();

    if (buttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        int tempDistance = 0;

        for (int i = 0; i < ui->slotListWidget->count(); i++)
        {
            SimpleKeyGrabberButton *button =
                ui->slotListWidget->item(i)->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
            JoyButtonSlot *tempbuttonslot = button->getValue();

            if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyDistance)
            {
                tempDistance += tempbuttonslot->getSlotCode();
            } else if (tempbuttonslot->getSlotMode() == JoyButtonSlot::JoyCycle)
            {
                tempDistance = 0;
            }
        }

        int testDistance = ui->distanceSpinBox->value();
        tempDistance += testDistance - buttonslot->getSlotCode();

        if (tempDistance <= 100)
        {
            tempbutton->setValue(testDistance, buttonslot->getSlotMode());

            QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection, Q_ARG(int, testDistance),
                                      Q_ARG(int, 0), Q_ARG(int, index),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, buttonslot->getSlotMode()));

            updateSlotsScrollArea(testDistance);
        }
    }
}

void AdvanceButtonDialog::updateWindowTitleButtonName()
{
    QString windTitleBtnName = QString().append(tr("Advanced").append(": ")).append(m_button->getPartialName(false, true));

    if (m_button->getParentSet()->getIndex() != 0)
    {
        int setIndex = m_button->getParentSet()->getRealIndex();
        windTitleBtnName.append(" [").append(tr("Set %1").arg(setIndex));
        QString setName = m_button->getParentSet()->getName();

        if (!setName.isEmpty())
            windTitleBtnName.append(": ").append(setName);

        windTitleBtnName.append("]");
    }

    setWindowTitle(windTitleBtnName);
}

void AdvanceButtonDialog::checkCycleResetWidgetStatus(bool enabled)
{
    if (enabled)
        ui->resetCycleDoubleSpinBox->setEnabled(true);
    else
        ui->resetCycleDoubleSpinBox->setEnabled(false);
}

void AdvanceButtonDialog::setButtonCycleResetInterval(double value)
{
    int milliseconds = value * 1000;
    m_button->setCycleResetTime(milliseconds);
}

void AdvanceButtonDialog::populateAutoResetInterval()
{
    double seconds = m_button->getCycleResetTime() / 1000.0;
    ui->resetCycleDoubleSpinBox->setValue(seconds);
}

void AdvanceButtonDialog::setButtonCycleReset(bool enabled)
{
    if (enabled)
    {
        m_button->setCycleResetStatus(true);

        if ((m_button->getCycleResetTime() == 0) && (ui->resetCycleDoubleSpinBox->value() > 0.0))
        {
            double current = ui->resetCycleDoubleSpinBox->value();
            setButtonCycleResetInterval(current);
        }
    } else
    {
        m_button->setCycleResetStatus(false);
    }
}

void AdvanceButtonDialog::resetTimeBoxes()
{
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
    disconnect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::updateActionTimeLabel);
    disconnect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::updateActionTimeLabel);

    disconnect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::checkSlotTimeUpdate);
    disconnect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AdvanceButtonDialog::checkSlotTimeUpdate);
}

void AdvanceButtonDialog::connectTimeBoxesEvents()
{
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::updateActionTimeLabel);

    connect(ui->actionHundredthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionSecondsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionMinutesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
    connect(ui->actionTenthsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AdvanceButtonDialog::checkSlotTimeUpdate);
}

void AdvanceButtonDialog::populateSetSelectionComboBox()
{
    ui->setSelectionComboBox->clear();
    ui->setSelectionComboBox->insertItem(0, tr("Disabled"));
    int currentIndex = 1;

    for (auto set = m_button->getParentSet()->getInputDevice()->getJoystick_sets().begin();
         set != m_button->getParentSet()->getInputDevice()->getJoystick_sets().end(); ++set)
    {
        int originset = set.key();
        if (m_button->getOriginSet() != originset)
        {
            QString selectedSetText = QString(tr("Select Set %1").arg(originset + 1));
            QString setName = set.value()->getName();

            if (!setName.isEmpty())
            {
                selectedSetText.append(" ").append("[");
                selectedSetText.append(setName).append("]").append(" ");
            }

            QString oneWayText = QString(selectedSetText).append(" ").append(tr("One Way"));
            QString twoWayText = QString(selectedSetText).append(" ").append(tr("Two Way"));
            QString whileHeldText = QString(selectedSetText).append(" ").append(tr("While Held"));

            QStringList setChoices = QStringList();
            setChoices.append(oneWayText);
            setChoices.append(twoWayText);
            setChoices.append(whileHeldText);

            ui->setSelectionComboBox->insertItems(currentIndex, setChoices);
            currentIndex += 3;
        }

        originset++;
    }
}

void AdvanceButtonDialog::populateSlotSetSelectionComboBox()
{
    ui->slotSetChangeComboBox->clear();
    int current_box_index = 0;

    for (auto set = m_button->getParentSet()->getInputDevice()->getJoystick_sets().begin();
         set != m_button->getParentSet()->getInputDevice()->getJoystick_sets().end(); ++set)
    {
        int originset = set.key();
        if (m_button->getOriginSet() != originset)
        {
            QString selectedSetSlotText = QString(tr("Select Set %1").arg(originset + 1));
            QString setName = set.value()->getName();

            if (!setName.isEmpty())
            {
                selectedSetSlotText.append(" ").append("[");
                selectedSetSlotText.append(setName).append("]").append(" ");
            }

            ui->slotSetChangeComboBox->insertItem(current_box_index, selectedSetSlotText, QVariant(originset));
            current_box_index++;
        }

        originset++;
    }
}

void AdvanceButtonDialog::findTurboModeComboIndex()
{
    JoyButton::TurboMode currentTurboMode = m_button->getTurboMode();

    switch (static_cast<int>(currentTurboMode))
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
    switch (value)
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
{ // It can be used as reusable code
    insertSlot();
}

void AdvanceButtonDialog::showSelectProfileWind(QListWidgetItem *item, QString &firstChoiceProfile)
{
    int index = ui->slotListWidget->row(item);
    AntiMicroSettings *settings = m_button->getParentSet()->getInputDevice()->getSettings();
    QString preferredDir, profileName;

    if (firstChoiceProfile.size() <= 0)
    {
        preferredDir = PadderCommon::preferredProfileDir(settings);
        profileName =
            QFileDialog::getOpenFileName(this, tr("Choose Profile"), preferredDir, tr("Config Files (*.amgp *.xml)"));
    } else
    {
        profileName = firstChoiceProfile;
    }

    if (!profileName.isEmpty())
    {
        firstChoiceProfile = profileName;
        SimpleKeyGrabberButton *button = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>();
        button->setValue(profileName, JoyButtonSlot::JoyLoadProfile);

        QMetaObject::invokeMethod(&helper, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(JoyButtonSlot *, button->getValue()), Q_ARG(int, index));

        button->setToolTip(profileName);
        updateSlotsScrollArea(0);
    }
}

bool AdvanceButtonDialog::anySelectedNotKeybSlot()
{
    for (auto item : ui->slotListWidget->selectedItems())
    {
        auto slotMode = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>()->getValue()->getSlotMode();

        if ((slotMode != JoyButtonSlot::JoySlotInputAction::JoyMix) &&
            (slotMode != JoyButtonSlot::JoySlotInputAction::JoyKeyboard))
        {
            return true;
        }
    }

    return false;
}

bool AdvanceButtonDialog::selectedNotMixSlot()
{
    for (auto item : ui->slotListWidget->selectedItems())
    {
        auto slotMode = item->data(Qt::UserRole).value<SimpleKeyGrabberButton *>()->getValue()->getSlotMode();

        if (slotMode != JoyButtonSlot::JoySlotInputAction::JoyMix)
        {
            return true;
        }
    }

    return false;
}

void AdvanceButtonDialog::showFindExecutableWindow(bool)
{
    QString preferredPath = QDir::homePath();

    QString execWindFilepath = QFileDialog::getOpenFileName(this, tr("Choose Executable"), preferredPath);
    ui->execLineEdit->setText(execWindFilepath);
}

void AdvanceButtonDialog::changeSlotTypeDisplay(int index)
{
    switch (index)
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
    switch (index)
    {
    case 0:
        ui->slotTypeHelpLabel->setText(tr("Insert a new blank slot."));
        break;

    case 1:
        ui->slotTypeHelpLabel->setText(tr("Slots past a Cycle action will be executed "
                                          "on the next button press. Multiple cycles can be added "
                                          "in order to create partitions in a sequence."));
        break;

    case 2:
        ui->slotTypeHelpLabel->setText(tr("Delays the time that the next slot is activated "
                                          "by the time specified. Slots activated before the "
                                          "delay will remain active after the delay time "
                                          "has passed."));
        break;

    case 3:
        ui->slotTypeHelpLabel->setText(tr("Distance action specifies that the slots afterwards "
                                          "will only be executed when an axis is moved "
                                          "a certain range past the designated dead zone."));
        break;

    case 4:
        ui->slotTypeHelpLabel->setText(tr("Execute program when slot is activated."));
        break;

    case 5:
        ui->slotTypeHelpLabel->setText(tr("Insert a hold action. Slots after the action will only be "
                                          "executed if the button is held past the interval specified."));
        break;

    case 6:
        ui->slotTypeHelpLabel->setText(tr("Chose a profile to load when this slot is activated."));
        break;

    case 7:
        ui->slotTypeHelpLabel->setText(tr("Mouse mod action will modify all mouse speed settings "
                                          "by a specified percentage while the action is being processed. "
                                          "This can be useful for slowing down the mouse while "
                                          "sniping."));
        break;

    case 8:
        ui->slotTypeHelpLabel->setText(tr("Insert a pause that occurs in between key presses."));
        break;

    case 9:
        ui->slotTypeHelpLabel->setText(tr("Specify the time that keys past this slot should be "
                                          "held down."));
        break;

    case 10:
        ui->slotTypeHelpLabel->setText(tr("Insert a release action. Slots after the action will only be "
                                          "executed after a button release if the button was held "
                                          "past the interval specified."));
        break;

    case 11:
        ui->slotTypeHelpLabel->setText(tr("Change to selected set once slot is activated."));
        break;

    case 12:
        ui->slotTypeHelpLabel->setText(tr("Full string will be typed when a "
                                          "slot is activated."));
        break;
    }
}

int AdvanceButtonDialog::getOldRow() const { return oldRow; }

JoyButton *AdvanceButtonDialog::getButton() const { return m_button; }

AdvanceButtonDialogHelper const &AdvanceButtonDialog::getHelper() { return helper; }

AdvanceButtonDialogHelper &AdvanceButtonDialog::getHelperLocal() { return helper; }
