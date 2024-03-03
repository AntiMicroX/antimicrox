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

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"

#include "advancebuttondialog.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"
#include "keyboard/virtualkeyboardmousewidget.h"
#include "quicksetdialog.h"

#include "antkeymapper.h"
#include "common.h"
#include "event.h"
#include "eventhandlerfactory.h"
#include "inputdevice.h"
#include "setjoystick.h"

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPointer>
#include <QWidget>
#include <QtGlobal>

#ifdef Q_OS_WIN
    #include "winextras.h"
    #include <qt_windows.h>
#endif

ButtonEditDialog *ButtonEditDialog::instance = nullptr;

ButtonEditDialog::ButtonEditDialog(InputDevice *joystick, bool isNumKeypad, QWidget *parent)
    : QDialog(parent, Qt::Window)
    , helper()
    , ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);

    withoutQuickSetDialog = false;
    m_isNumKeypad = isNumKeypad;
    ui->attachNumKeypadCheckbox->setChecked(isNumKeypad);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Choose your keyboard key"));
    update();

    instance = this;
    this->joystick = joystick;
    lastJoyButton = nullptr;
    currentQuickDialog = nullptr;

    SetJoystick *currentset = joystick->getActiveSetJoystick();
    currentset->release();
    joystick->resetButtonDownCount();

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);

    ignoreRelease = false;

    setupVirtualKeyboardMouseTabWidget();

    connect(qApp, &QApplication::focusChanged, this, &ButtonEditDialog::checkForKeyboardWidgetFocus);
    connect(this, &ButtonEditDialog::keyGrabbed, this, &ButtonEditDialog::processSlotAssignment);
    connect(this, &ButtonEditDialog::selectionCleared, this,
            &ButtonEditDialog::clearButtonSlots); //  used to clear button sets

    connect(ui->toggleCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeToggleSetting);
    connect(ui->turboCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeTurboSetting);
    connect(ui->attachNumKeypadCheckbox, &QCheckBox::clicked, this, &ButtonEditDialog::changeNumKeypadSetting);
    connect(ui->advancedPushButton, &QPushButton::clicked, this, &ButtonEditDialog::openAdvancedDialog);
    refreshForLastBtn();
}

ButtonEditDialog::ButtonEditDialog(JoyButton *button, InputDevice *joystick, bool isNumKeypad, QWidget *parent)
    : ButtonEditDialog(joystick, isNumKeypad, parent)
{

    withoutQuickSetDialog = true;

    lastJoyButton = button;

    setupVirtualKeyboardMouseTabWidget();
    refreshForLastBtn();
}

// for tests
ButtonEditDialog::ButtonEditDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , helper()
    , ui(new Ui::ButtonEditDialog)
{
}

ButtonEditDialog *ButtonEditDialog::getInstance() { return instance; }

void ButtonEditDialog::checkForKeyboardWidgetFocus(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);
    Q_UNUSED(now);

    if (ui->virtualKeyMouseTabWidget->hasFocus() && ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        grabKeyboard();
    } else
    {
        releaseKeyboard();
    }
}

ButtonEditDialog::~ButtonEditDialog()
{
    instance = nullptr;

    delete ui;
}

void ButtonEditDialog::keyPressEvent(QKeyEvent *event)
{
    bool ignore = false;
    // Ignore the following keys that might
    // trigger an event in QDialog::keyPressEvent
    switch (event->key())
    {
    case Qt::Key_Escape:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        ignore = true;
        break;
    }
    default:
        break;
    }

    if (!ignore)
        QDialog::keyPressEvent(event);
}

void ButtonEditDialog::setupVirtualKeyboardMouseTabWidget()
{

    PadderCommon::inputDaemonMutex.lock();

    ui->virtualKeyMouseTabWidget->hide();
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget =
        new VirtualKeyboardMouseWidget(joystick, &helper, m_isNumKeypad, currentQuickDialog, lastJoyButton, this);
    ui->verticalLayout->insertWidget(1, ui->virtualKeyMouseTabWidget);

    PadderCommon::inputDaemonMutex.unlock();

    connect(ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::selectionCleared, this,
            &ButtonEditDialog::refreshSlotSummaryLabel);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget,
            &VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget,
            &VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections);
}

void ButtonEditDialog::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "It's keyrelease event";

    if (ui->actionNameLineEdit->hasFocus() || ui->buttonNameLineEdit->hasFocus())
    {
        QDialog::keyReleaseEvent(event);
    } else if (ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        int controlcode = event->nativeScanCode();
        int virtualactual = event->nativeVirtualKey();

        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        int finalvirtual = 0;
        int checkalias = 0;
#ifdef Q_OS_WIN
    #ifdef WITH_VMULTI
        if (handler->getIdentifier() == "vmulti")
        {
            finalvirtual = WinExtras::correctVirtualKey(controlcode, virtualactual);
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);

            // unsigned int tempQtKey = nativeWinKeyMapper.returnQtKey(finalvirtual);
            QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
            unsigned int tempQtKey = 0;
            if (nativeWinKeyMapper)
            {
                tempQtKey = nativeWinKeyMapper->returnQtKey(finalvirtual);
            }

            if (tempQtKey > 0)
            {
                finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempQtKey);
                checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
            } else
            {
                finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
            }
        }
    #endif

        BACKEND_ELSE_IF(handler->getIdentifier() == "sendinput")
        {
            // Find more specific virtual key (VK_SHIFT -> VK_LSHIFT)
            // by checking for extended bit in scan code.
            finalvirtual = WinExtras::correctVirtualKey(controlcode, virtualactual);
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual, controlcode);
        }
#else
    #if defined(WITH_X11)

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            // Obtain group 1 X11 keysym. Removes effects from modifiers.
            finalvirtual = X11KeyCodeToX11KeySym(controlcode);

        #ifdef WITH_UINPUT
            if (handler->getIdentifier() == "uinput")
            {
                // Find Qt Key corresponding to X11 KeySym.
                Q_ASSERT(AntKeyMapper::getInstance()->hasNativeKeyMapper());
                QtKeyMapperBase *x11KeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
                Q_ASSERT(x11KeyMapper != nullptr);
                checkalias = x11KeyMapper->returnQtKey(finalvirtual);
                finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(
                    checkalias); // Find corresponding Linux input key for the Qt key.
            }
        #endif

        #ifdef WITH_XTEST
            BACKEND_ELSE_IF(handler->getIdentifier() == "xtest")
            {
                // Check for alias against group 1 keysym.
                checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
            }
        #endif
        } else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

    #else
        finalvirtual = 0;
        checkalias = 0;
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        } else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

    #endif
#endif

        if (!ignoreRelease && (event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_X))
        {
            controlcode = 0;
            ignoreRelease = true;
            emit selectionCleared();
        } else if (!ignoreRelease && (controlcode <= 0))
        {
            controlcode = 0;
        } else if (ignoreRelease)
        {
            controlcode = 0;
            ignoreRelease = false;
        }

        if (controlcode > 0 && (checkalias > 0) && (finalvirtual > 0))
        {
            JoyButtonSlot *tempslot = new JoyButtonSlot(finalvirtual, checkalias, JoyButtonSlot::JoyKeyboard, this);
            emit keyGrabbed(tempslot);
        } else if ((controlcode > 0) && (virtualactual > 0))
        {
            JoyButtonSlot *tempslot = new JoyButtonSlot(virtualactual, JoyButtonSlot::JoyKeyboard, this);
            emit keyGrabbed(tempslot);
        } else
        {
            QDialog::keyReleaseEvent(event);
        }
    } else
    {
        QDialog::keyReleaseEvent(event);
    }
}

void ButtonEditDialog::refreshSlotSummaryLabel()
{
    if (lastJoyButton != nullptr)
        ui->slotSummaryLabel->setText(lastJoyButton->getSlotsString().replace("&", "&&"));
    else
        ui->slotSummaryLabel->setText(tr("No button"));
}

void ButtonEditDialog::changeToggleSetting()
{
    if (lastJoyButton != nullptr)
        lastJoyButton->setToggle(ui->toggleCheckBox->isChecked());
    else
        QMessageBox::information(
            this, tr("Last button"),
            tr("To change settings for last button, it must be at least one assignment from keyboard to gamepad"));
}

void ButtonEditDialog::changeTurboSetting()
{
    if (lastJoyButton != nullptr)
        lastJoyButton->setUseTurbo(ui->turboCheckBox->isChecked());
    else
        QMessageBox::information(
            this, tr("Last button"),
            tr("To change settings of turbo for last button, it must be at least one assignment from keyboard to gamepad"));
}

void ButtonEditDialog::changeNumKeypadSetting()
{
    m_isNumKeypad = ui->attachNumKeypadCheckbox->isChecked();
    setupVirtualKeyboardMouseTabWidget();
    refreshForLastBtn();
    joystick->getSettings()->setValue("AttachNumKeypad", m_isNumKeypad ? "1" : "0");
}

void ButtonEditDialog::openAdvancedDialog()
{
    if (lastJoyButton != nullptr)
    {
        if (lastJoyButton->getAssignedSlots()->count() > 0)
        {
            AdvanceButtonDialog *dialog = new AdvanceButtonDialog(lastJoyButton, this);
            dialog->show();

            // Disconnect event to allow for placing slot to AdvanceButtonDialog
            disconnect(this, &ButtonEditDialog::keyGrabbed, nullptr, nullptr);
            disconnect(this, &ButtonEditDialog::selectionCleared, nullptr, nullptr);
            disconnect(this, &ButtonEditDialog::selectionFinished, nullptr, nullptr);

            connect(dialog, &AdvanceButtonDialog::finished, ui->virtualKeyMouseTabWidget,
                    &VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections);
            connect(dialog, &AdvanceButtonDialog::finished, ui->virtualKeyMouseTabWidget,
                    &VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections);
            connect(dialog, &AdvanceButtonDialog::finished, this, &ButtonEditDialog::closedAdvancedDialog);
            connect(dialog, &AdvanceButtonDialog::turboButtonEnabledChange, this, &ButtonEditDialog::setTurboButtonEnabled);

            connect(this, &ButtonEditDialog::sendTempSlotToAdvanced, dialog, &AdvanceButtonDialog::placeNewSlot);
            connect(this, &ButtonEditDialog::keyGrabbed, dialog, &AdvanceButtonDialog::placeNewSlot);
            connect(this, &ButtonEditDialog::selectionCleared, dialog, &AdvanceButtonDialog::clearAllSlots);
            connect(ui->virtualKeyMouseTabWidget,
                    static_cast<void (VirtualKeyboardMouseWidget::*)(JoyButtonSlot *)>(
                        &VirtualKeyboardMouseWidget::selectionMade),
                    dialog, &AdvanceButtonDialog::placeNewSlot);
            connect(ui->virtualKeyMouseTabWidget,
                    static_cast<void (VirtualKeyboardMouseWidget::*)(int, int)>(&VirtualKeyboardMouseWidget::selectionMade),
                    this, &ButtonEditDialog::createTempSlot);
            connect(ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::selectionCleared, dialog,
                    &AdvanceButtonDialog::clearAllSlots);

            connect(this, &ButtonEditDialog::finished, dialog, &AdvanceButtonDialog::close);
            emit advancedDialogOpened();

        } else
        {
            QMessageBox::information(this, tr("No choice"),
                                     tr("Before you open window with advanced settings, you have to choice a key"));
        }

    } else
    {
        QMessageBox::information(
            this, tr("Last button"),
            tr("To open advanced dialog, it's needed to map at least one button from keyboard to gamepad"));
    }
}

void ButtonEditDialog::createTempSlot(int keycode, int alias)
{
    JoyButtonSlot *slot = new JoyButtonSlot(keycode, alias, JoyButtonSlot::JoyKeyboard, this);
    emit sendTempSlotToAdvanced(slot);
}

void ButtonEditDialog::checkTurboSetting(bool state)
{
    if (lastJoyButton != nullptr)
    {
        if (lastJoyButton->containsSequence())
        {
            ui->turboCheckBox->setChecked(false);
            ui->turboCheckBox->setEnabled(false);
        } else
        {
            ui->turboCheckBox->setChecked(state);
            ui->turboCheckBox->setEnabled(true);
        }
    }
}

void ButtonEditDialog::setTurboButtonEnabled(bool state) { ui->turboCheckBox->setEnabled(state); }

void ButtonEditDialog::closedAdvancedDialog()
{
    refreshForLastBtn();

    disconnect(ui->virtualKeyMouseTabWidget,
               static_cast<void (VirtualKeyboardMouseWidget::*)(int, int)>(&VirtualKeyboardMouseWidget::selectionMade), this,
               nullptr);

    // Re-connect previously disconnected event
    connect(this, &ButtonEditDialog::keyGrabbed, this, &ButtonEditDialog::processSlotAssignment);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::clearButtonSlots);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::sendSelectionFinished);
}

void ButtonEditDialog::processSlotAssignment(JoyButtonSlot *tempslot)
{
    if (withoutQuickSetDialog)
    {
        invokeMethodLastBtn(lastJoyButton, &helper, "setAssignedSlot", tempslot->getSlotCode(), tempslot->getSlotCodeAlias(),
                            -1, tempslot->getSlotMode(), false, false, Qt::QueuedConnection, Qt::QueuedConnection,
                            Qt::QueuedConnection);
    } else
    {
        if ((currentQuickDialog == nullptr) && (!buttonEventInterval.isValid() || (buttonEventInterval.elapsed() > 1000)))
        {
            // for better security, force pausing for 1 sec between key presses,
            // because mapped joystick buttons and axes become keys too
            // it's good for oversensitive buttons and axes, which can
            // create QuickSetDialog windows many times for one key

            if (!buttonEventInterval.isValid())
                buttonEventInterval.start();
            else
                buttonEventInterval.restart();

            currentQuickDialog =
                new QuickSetDialog(joystick, &helper, "setAssignedSlot", tempslot->getSlotCode(),
                                   tempslot->getSlotCodeAlias(), -1, tempslot->getSlotMode(), false, false, this);
            currentQuickDialog->show();
            connect(currentQuickDialog, &QuickSetDialog::finished, this, &ButtonEditDialog::nullifyDialogPointer);
        }
    }

    tempslot->deleteLater();
}

void ButtonEditDialog::clearButtonSlots()
{
    if (lastJoyButton != nullptr)
        QMetaObject::invokeMethod(lastJoyButton, "clearSlotsEventReset", Q_ARG(bool, false));
    else
        QMessageBox::information(this, tr("Last button"),
                                 tr("Slots for button couldn't be cleared, because there was not any set button from "
                                    "keyboard for gamepad. Map at least one button from keyboard to gamepad"));
}

void ButtonEditDialog::sendSelectionFinished() { emit selectionFinished(); }

void ButtonEditDialog::updateWindowTitleButtonName()
{
    if (lastJoyButton != nullptr)
    {
        QString temp = QString(tr("As last gamepad button has been set"))
                           .append(" \"")
                           .append(lastJoyButton->getPartialName(false, true))
                           .append("\" ");

        if (lastJoyButton->getParentSet()->getIndex() != 0)
        {
            int setIndex = lastJoyButton->getParentSet()->getRealIndex();
            temp.append(" [").append(tr("Index %1").arg(setIndex));
            QString setName = lastJoyButton->getParentSet()->getName();

            if (!setName.isEmpty())
                temp.append(": ").append(setName);

            temp.append("]");
        }

        setWindowTitle(temp);

    } else
    {
        setWindowTitle(tr("Choose your keyboard key"));
    }
}

void ButtonEditDialog::nullifyDialogPointer()
{
    if (currentQuickDialog != nullptr)
    {
        lastJoyButton = currentQuickDialog->getLastPressedButton();
        currentQuickDialog = nullptr;
        emit buttonDialogClosed();
    }

    refreshForLastBtn();
}

void ButtonEditDialog::refreshForLastBtn()
{
    if (lastJoyButton != nullptr)
    {
        ui->slotSummaryLabel->setText(lastJoyButton->getSlotsString());

        updateWindowTitleButtonName();

        ui->toggleCheckBox->setChecked(lastJoyButton->getToggleState());
        ui->turboCheckBox->setChecked(lastJoyButton->isUsingTurbo());

        if (!lastJoyButton->getActionName().isEmpty())
            ui->actionNameLineEdit->setText(lastJoyButton->getActionName());

        if (!lastJoyButton->getButtonName().isEmpty())
            ui->buttonNameLineEdit->setText(lastJoyButton->getButtonName());

        if (lastJoyButton != nullptr)
        {
            QListIterator<JoyButtonSlot *> iter(*(lastJoyButton->getAssignedSlots()));

            ui->virtualKeyMouseTabWidget->disableMouseSettingButton();

            while (iter.hasNext())
            {
                JoyButtonSlot *buttonslot = iter.next();

                switch (buttonslot->getSlotMode())
                {
                case JoyButtonSlot::JoyMouseMovement:
                case JoyButtonSlot::JoyMouseButton:
                    ui->virtualKeyMouseTabWidget->enableMouseSettingButton();
                    break;

                default:
                    break;
                }
            }
        }

        connect(ui->actionNameLineEdit, &QLineEdit::textEdited, lastJoyButton, &JoyButton::setActionName);
        connect(ui->buttonNameLineEdit, &QLineEdit::textEdited, lastJoyButton, &JoyButton::setButtonName);

        connect(lastJoyButton, &JoyButton::toggleChanged, ui->toggleCheckBox, &QCheckBox::setChecked);
        connect(lastJoyButton, &JoyButton::turboChanged, this, &ButtonEditDialog::checkTurboSetting);
        connect(lastJoyButton, &JoyButton::slotsChanged, this, &ButtonEditDialog::refreshSlotSummaryLabel);
        connect(lastJoyButton, &JoyButton::buttonNameChanged, this, &ButtonEditDialog::updateWindowTitleButtonName);
    }

    update();
}

void ButtonEditDialog::invokeMethodLastBtn(JoyButton *lastJoyBtn, ButtonEditDialogHelper *helper, const char *invokeString,
                                           int code, int alias, int index, JoyButtonSlot::JoySlotInputAction mode,
                                           bool withClear, bool withTrue, Qt::ConnectionType connTypeForAlias,
                                           Qt::ConnectionType connTypeForNothing, Qt::ConnectionType connTypeForAll)
{
    QPointer<JoyButton> lastBtn = lastJoyBtn;
    if (helper != nullptr)
        helper = new ButtonEditDialogHelper();

    QPointer<ButtonEditDialogHelper> btnEditDHelper = helper;

    if (lastJoyBtn != nullptr)
    {
        helper->setThisButton(lastJoyBtn);
        helper->moveToThread(lastJoyBtn->thread());

        if (withClear)
            QMetaObject::invokeMethod(lastJoyBtn, "clearSlotsEventReset", Q_ARG(bool, withTrue));

        // when alias exists but not index
        if ((alias != -1) && (index == -1))
        {
            QMetaObject::invokeMethod(helper, invokeString, connTypeForAlias, Q_ARG(int, code), Q_ARG(int, alias),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

            // when alias doesn't exists and index too
        } else if ((alias == -1) && (index == -1))
        {
            QMetaObject::invokeMethod(helper, invokeString, connTypeForNothing, Q_ARG(int, code),
                                      Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

            // when all exist (code, alias, index)
        } else
        {
            if (lastJoyBtn->isPartVDPad())
                connTypeForAll = Qt::BlockingQueuedConnection;

            QMetaObject::invokeMethod(helper, invokeString, connTypeForAll, Q_ARG(int, code), Q_ARG(int, alias),
                                      Q_ARG(int, index), Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));
        }
    }

    if (lastBtn.isNull())
        lastBtn.clear();
    if (btnEditDHelper.isNull())
        btnEditDHelper.clear();
}

JoyButton *ButtonEditDialog::getLastJoyButton() { return lastJoyButton; }

void ButtonEditDialog::setUpLastJoyButton(JoyButton *newButton) { lastJoyButton = newButton; }

ButtonEditDialogHelper &ButtonEditDialog::getHelperLocal() { return helper; }
