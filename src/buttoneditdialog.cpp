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

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"

#include "messagehandler.h"
#include "joybutton.h"
#include "keyboard/virtualkeyboardmousewidget.h"
#include "advancebuttondialog.h"
#include "inputdevice.h"
#include "quicksetdialog.h"

#include "event.h"
#include "antkeymapper.h"
#include "eventhandlerfactory.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "common.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include "winextras.h"

#elif defined(Q_OS_UNIX)
#include <QApplication>
#endif


#include <QDebug>
#include <QPointer>
#include <QtGlobal>
#include <QWidget>
#include <QKeyEvent>
#include <QMessageBox>


ButtonEditDialog* ButtonEditDialog::instance = nullptr;


ButtonEditDialog::ButtonEditDialog(InputDevice* joystick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    helper(),
    ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(trUtf8("Choose your keyboard key"));
    ui->advancedPushButton->setEnabled(false);
    update();

    instance = this;
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->joystick = joystick;
    lastJoyButton = nullptr;
    currentQuickDialog = nullptr;

    SetJoystick *currentset = joystick->getActiveSetJoystick();
    currentset->release();
    joystick->resetButtonDownCount();

    setMinimumHeight(460);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);

    ignoreRelease = false;

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in ButtonEditDialog";
    #endif

    PadderCommon::inputDaemonMutex.lock();

    ui->virtualKeyMouseTabWidget->hide();
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget = new VirtualKeyboardMouseWidget(joystick ,&helper, currentQuickDialog, this);
    ui->verticalLayout->insertWidget(1, ui->virtualKeyMouseTabWidget);

    PadderCommon::inputDaemonMutex.unlock();

    connect(qApp, &QApplication::focusChanged, this, &ButtonEditDialog::checkForKeyboardWidgetFocus);

    connect(ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::selectionCleared, this, &ButtonEditDialog::refreshSlotSummaryLabel);

    connect(this, &ButtonEditDialog::keyGrabbed, this, &ButtonEditDialog::processSlotAssignment);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::clearButtonSlots);

    connect(ui->toggleCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeToggleSetting);
    connect(ui->turboCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeTurboSetting);
    connect(ui->advancedPushButton, &QPushButton::clicked, this, &ButtonEditDialog::openAdvancedDialog);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections);

    refreshForLastBtn();
}


ButtonEditDialog::ButtonEditDialog(JoyButton* button, InputDevice* joystick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(trUtf8("Choose your keyboard key"));
    ui->advancedPushButton->setEnabled(false);
    update();

    instance = this;
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastJoyButton = button;
    this->joystick = joystick;
    currentQuickDialog = nullptr;

    SetJoystick *currentset = joystick->getActiveSetJoystick();
    currentset->release();
    joystick->resetButtonDownCount();

    setMinimumHeight(460);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);

    ignoreRelease = false;

    PadderCommon::inputDaemonMutex.lock();

    ui->virtualKeyMouseTabWidget->hide();
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget = new VirtualKeyboardMouseWidget(joystick, &helper, currentQuickDialog, this);
    ui->verticalLayout->insertWidget(1, ui->virtualKeyMouseTabWidget);

    PadderCommon::inputDaemonMutex.unlock();

    connect(qApp, &QApplication::focusChanged, this, &ButtonEditDialog::checkForKeyboardWidgetFocus);


    connect(ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::selectionCleared, this, &ButtonEditDialog::refreshSlotSummaryLabel);

    connect(this, &ButtonEditDialog::keyGrabbed, this, &ButtonEditDialog::processSlotAssignment);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::clearButtonSlots); //  used to clear button sets

    connect(ui->toggleCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeToggleSetting);
    connect(ui->turboCheckBox, &QCheckBox::clicked, this, &ButtonEditDialog::changeTurboSetting);
    connect(ui->advancedPushButton, &QPushButton::clicked, this, &ButtonEditDialog::openAdvancedDialog);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections);
    connect(this, &ButtonEditDialog::advancedDialogOpened, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections);

    refreshForLastBtn();
}


ButtonEditDialog* ButtonEditDialog::getInstance()
{
    return instance;
}


void ButtonEditDialog::checkForKeyboardWidgetFocus(QWidget *old, QWidget *now)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(old);
    Q_UNUSED(now);

    if (ui->virtualKeyMouseTabWidget->hasFocus() &&
        ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        grabKeyboard();
    }
    else
    {
        releaseKeyboard();
    }
}

ButtonEditDialog::~ButtonEditDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (instance != nullptr) {
        instance = nullptr;
    }

    delete ui;

}


void ButtonEditDialog::keyPressEvent(QKeyEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool ignore = false;
    // Ignore the following keys that might
    // trigger an event in QDialog::keyPressEvent
    switch(event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Right:
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_Left:
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            ignore = true;
            break;
        }
    }

    if (!ignore)
    {
        QDialog::keyPressEvent(event);
    }
}

void ButtonEditDialog::keyReleaseEvent(QKeyEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    qDebug() << "It's keyrelease event";

    if (ui->actionNameLineEdit->hasFocus() || ui->buttonNameLineEdit->hasFocus())
    {
        QDialog::keyReleaseEvent(event);
    }
    else if (ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        int controlcode = event->nativeScanCode();
        int virtualactual = event->nativeVirtualKey();

        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

#ifdef Q_OS_WIN
        int finalvirtual = 0;
        int checkalias = 0;

  #ifdef WITH_VMULTI
      if (handler->getIdentifier() == "vmulti")
      {
          finalvirtual = WinExtras::correctVirtualKey(controlcode, virtualactual);
          checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);

          QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
          int tempQtKey = 0;
          if (nativeWinKeyMapper != nullptr)
          {
              tempQtKey = nativeWinKeyMapper->returnQtKey(finalvirtual);
          }

          if (tempQtKey > 0)
          {
              finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempQtKey);
              checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
          }
          else
          {
              finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
          }
      }

  #endif

      BACKEND_ELSE_IF (handler->getIdentifier() == "sendinput")
      {
          // Find more specific virtual key (VK_SHIFT -> VK_LSHIFT)
          // by checking for extended bit in scan code.
          finalvirtual = WinExtras::correctVirtualKey(controlcode, virtualactual);
          checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual, controlcode);
      }

#elif defined(Q_OS_UNIX)

    #if defined(WITH_X11)
        int finalvirtual = 0;
        int checkalias = 0;


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
            // Find corresponding Linux input key for the Qt key.
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(checkalias);
        }
        #endif

        #ifdef WITH_XTEST
        BACKEND_ELSE_IF (handler->getIdentifier() == "xtest")
        {
            // Check for alias against group 1 keysym.
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

        #endif

        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }


    #else
        int finalvirtual = 0;
        int checkalias = 0;
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
        checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = controlcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

    #endif

#endif

        if (!ignoreRelease)
        {
            if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_X))
            {
                controlcode = 0;
                ignoreRelease = true;
                emit selectionCleared();
            }
            else if (controlcode <= 0)
            {
                controlcode = 0;
            }
        }
        else
        {
            controlcode = 0;
            ignoreRelease = false;
        }


        if (controlcode > 0)
        {
            if ((checkalias > 0) && (finalvirtual > 0))
            {
                JoyButtonSlot *tempslot = new JoyButtonSlot(finalvirtual, checkalias, JoyButtonSlot::JoyKeyboard, this);
                emit keyGrabbed(tempslot);
            }
            else if (virtualactual > 0)
            {
                JoyButtonSlot *tempslot = new JoyButtonSlot(virtualactual, JoyButtonSlot::JoyKeyboard, this);
                emit keyGrabbed(tempslot);
            }
            else
            {
                QDialog::keyReleaseEvent(event);
            }
        }
        else
        {
            QDialog::keyReleaseEvent(event);
        }
    }
    else
    {
        QDialog::keyReleaseEvent(event);
    }
}

void ButtonEditDialog::refreshSlotSummaryLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        if (lastJoyButton != nullptr) ui->slotSummaryLabel->setText(lastJoyButton->getSlotsString().replace("&", "&&"));
        else ui->slotSummaryLabel->setText(trUtf8("No button"));
}

void ButtonEditDialog::changeToggleSetting()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

       if (lastJoyButton != nullptr) lastJoyButton->setToggle(ui->toggleCheckBox->isChecked());
       else QMessageBox::information(this, trUtf8("Last button"), trUtf8("To change settings for last button, it must be at least one assignment from keyboard to gamepad"));
}

void ButtonEditDialog::changeTurboSetting()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        if (lastJoyButton != nullptr) lastJoyButton->setUseTurbo(ui->turboCheckBox->isChecked());
        else QMessageBox::information(this, trUtf8("Last button"), trUtf8("To change settings of turbo for last button, it must be at least one assignment from keyboard to gamepad"));
}

void ButtonEditDialog::openAdvancedDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->advancedPushButton->setEnabled(false);

    if (lastJoyButton != nullptr) {
    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(lastJoyButton, this);
    dialog->show();

    // Disconnect event to allow for placing slot to AdvanceButtonDialog
    disconnect(this, &ButtonEditDialog::keyGrabbed, 0, 0);
    disconnect(this, &ButtonEditDialog::selectionCleared, 0, 0);
    disconnect(this, &ButtonEditDialog::selectionFinished, 0, 0);

    connect(dialog, &AdvanceButtonDialog::finished, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections);
    connect(dialog, &AdvanceButtonDialog::finished, ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections);
    connect(dialog, &AdvanceButtonDialog::finished, this, &ButtonEditDialog::closedAdvancedDialog);
    connect(dialog, &AdvanceButtonDialog::turboButtonEnabledChange, this, &ButtonEditDialog::setTurboButtonEnabled);

    connect(this, &ButtonEditDialog::sendTempSlotToAdvanced, dialog, &AdvanceButtonDialog::placeNewSlot);
    connect(this, &ButtonEditDialog::keyGrabbed, dialog, &AdvanceButtonDialog::placeNewSlot);
    connect(this, &ButtonEditDialog::selectionCleared, dialog, &AdvanceButtonDialog::clearAllSlots);
    connect(ui->virtualKeyMouseTabWidget, static_cast<void (VirtualKeyboardMouseWidget::*)(JoyButtonSlot*)>(&VirtualKeyboardMouseWidget::selectionMade), dialog, &AdvanceButtonDialog::placeNewSlot);
    connect(ui->virtualKeyMouseTabWidget, static_cast<void (VirtualKeyboardMouseWidget::*)(int,int)>(&VirtualKeyboardMouseWidget::selectionMade), this, &ButtonEditDialog::createTempSlot);
    connect(ui->virtualKeyMouseTabWidget, &VirtualKeyboardMouseWidget::selectionCleared, dialog, &AdvanceButtonDialog::clearAllSlots);

    connect(this, &ButtonEditDialog::finished, dialog, &AdvanceButtonDialog::close);
    emit advancedDialogOpened();

    } else {

        QMessageBox::information(this, trUtf8("Last button"), trUtf8("To open advanced dialog, it's needed to map at least one button from keyboard to gamepad"));
    }
}

void ButtonEditDialog::createTempSlot(int keycode, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *slot = new JoyButtonSlot(keycode, alias,
                                            JoyButtonSlot::JoyKeyboard, this);
    emit sendTempSlotToAdvanced(slot);
}

void ButtonEditDialog::checkTurboSetting(bool state)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (lastJoyButton != nullptr) {
    if (lastJoyButton->containsSequence())
    {
        ui->turboCheckBox->setChecked(false);
        ui->turboCheckBox->setEnabled(false);
    }
    else
    {
        ui->turboCheckBox->setChecked(state);
        ui->turboCheckBox->setEnabled(true);
    }

    getHelperLocal().setUseTurbo(state);

    }
}

void ButtonEditDialog::setTurboButtonEnabled(bool state)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->turboCheckBox->setEnabled(state);
}

void ButtonEditDialog::closedAdvancedDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->advancedPushButton->setEnabled(true);

    disconnect(ui->virtualKeyMouseTabWidget, static_cast<void (VirtualKeyboardMouseWidget::*)(int,int)>(&VirtualKeyboardMouseWidget::selectionMade), this, 0);

    // Re-connect previously disconnected event
    connect(this, &ButtonEditDialog::keyGrabbed, this, &ButtonEditDialog::processSlotAssignment);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::clearButtonSlots);
    connect(this, &ButtonEditDialog::selectionCleared, this, &ButtonEditDialog::sendSelectionFinished);

}

void ButtonEditDialog::processSlotAssignment(JoyButtonSlot *tempslot)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);


        if ((currentQuickDialog == nullptr) && (buttonEventInterval.isNull() || (buttonEventInterval.elapsed() > 1000)))
    {
        // for better security, force pausing for 1 sec between key presses,
        // because mapped joystick buttons and axes become keys too
        // it's good for oversensitive buttons and axes, which can
        // create QuickSetDialog windows many times for one key
        if (buttonEventInterval.isNull()) buttonEventInterval.start();
        else
            buttonEventInterval.restart();


        currentQuickDialog = new QuickSetDialog(joystick, &helper, "setAssignedSlot", tempslot->getSlotCode(), tempslot->getSlotCodeAlias(), -1, tempslot->getSlotMode(), false, false, this);

        currentQuickDialog->show();

        connect(currentQuickDialog, &QuickSetDialog::finished, this, &ButtonEditDialog::nullifyDialogPointer);
    }

    tempslot->deleteLater();
}

void ButtonEditDialog::clearButtonSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (lastJoyButton != nullptr)
        QMetaObject::invokeMethod(lastJoyButton, "clearSlotsEventReset", Q_ARG(bool, false));
    else
        QMessageBox::information(this, trUtf8("Last button"), trUtf8("Slots for button couldn't be cleared, because there was not any set button from keyboard for gamepad. Map at least one button from keyboard to gamepad"));

}

void ButtonEditDialog::sendSelectionFinished()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit selectionFinished();
}

void ButtonEditDialog::updateWindowTitleButtonName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (lastJoyButton != nullptr) {

        QString temp = QString(trUtf8("As last gamepad button has been set")).append(" \"").append(lastJoyButton->getPartialName(false, true)).append("\" ");

    if (lastJoyButton->getParentSet()->getIndex() != 0)
    {
        int setIndex = lastJoyButton->getParentSet()->getRealIndex();
        temp.append(" [").append(trUtf8("Index %1").arg(setIndex));
        QString setName = lastJoyButton->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);

    } else {

        setWindowTitle(trUtf8("Choose your keyboard key"));
    }

}


void ButtonEditDialog::nullifyDialogPointer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentQuickDialog != nullptr)
    {
        lastJoyButton = currentQuickDialog->getLastPressedButton();
        currentQuickDialog = nullptr;
        emit buttonDialogClosed();
    }

    refreshForLastBtn();
}


void ButtonEditDialog::refreshForLastBtn() {

    if (lastJoyButton != nullptr) {

        ui->advancedPushButton->setEnabled(true);


    ui->slotSummaryLabel->setText(lastJoyButton->getSlotsString());
    updateWindowTitleButtonName();

    ui->toggleCheckBox->setChecked(lastJoyButton->getToggleState());
    ui->turboCheckBox->setChecked(lastJoyButton->isUsingTurbo());

    if (!lastJoyButton->getActionName().isEmpty())
    {
        ui->actionNameLineEdit->setText(lastJoyButton->getActionName());
    }

    if (!lastJoyButton->getButtonName().isEmpty())
    {
        ui->buttonNameLineEdit->setText(lastJoyButton->getButtonName());
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


JoyButton* ButtonEditDialog::getLastJoyButton() {

    return lastJoyButton;
}

void ButtonEditDialog::setUpLastJoyButton(JoyButton * newButton) {

    lastJoyButton = newButton;
}

ButtonEditDialogHelper& ButtonEditDialog::getHelperLocal() {

    return helper;
}
