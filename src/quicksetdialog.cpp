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

#include "quicksetdialog.h"
#include "ui_quicksetdialog.h"

#include "messagehandler.h"
#include "setjoystick.h"
#include "buttoneditdialog.h"
#include "uihelpers/buttoneditdialoghelper.h"
#include "inputdevice.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joydpadbutton.h"
#include "joydpad.h"
#include "vdpad.h"

#include <QHash>
#include <QHashIterator>
#include <QWidget>
#include <QLabel>
#include <QDebug>


QuickSetDialog::QuickSetDialog(InputDevice *joystick, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;
    this->currentButtonDialog = nullptr;

    setWindowTitle(trUtf8("Quick Set %1").arg(joystick->getName()));

    SetJoystick *currentset = joystick->getActiveSetJoystick();
    currentset->release();
    joystick->resetButtonDownCount();

    QString temp = ui->joystickDialogLabel->text();
    temp = temp.arg(joystick->getSDLName()).arg(joystick->getName());
    ui->joystickDialogLabel->setText(temp);

    for (int i = 0; i < currentset->getNumberSticks(); i++)
    {
        JoyControlStick *stick = currentset->getJoyStick(i);
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stickButtons);

        while (iter.hasNext())
        {
            JoyControlStickButton *stickbutton = iter.next().value();

            if (stick->getJoyMode() != JoyControlStick::EightWayMode)
            {
                if ((stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickLeftUp)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickRightUp)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickLeftDown)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickRightDown)))
                {
                    connect(stickbutton, &JoyControlStickButton::clicked, this, [this, stickbutton]() {
                        showStickButtonDialog(stickbutton);
                    });
                }
            }
            else
            {
                connect(stickbutton, &JoyControlStickButton::clicked, this, [this, stickbutton]() {
                    showStickButtonDialog(stickbutton);
                });
            }

            if (!stickbutton->getIgnoreEventState())
            {
                stickbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i = 0; i < currentset->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            JoyAxisButton *paxisbutton = axis->getPAxisButton();

            connect(naxisbutton, &JoyAxisButton::clicked, this, [this, naxisbutton]() {
                showAxisButtonDialog(naxisbutton);
            });

            connect(paxisbutton, &JoyAxisButton::clicked, this, [this, paxisbutton]() {
                showAxisButtonDialog(paxisbutton);
            });

            if (!naxisbutton->getIgnoreEventState())
            {
                naxisbutton->setIgnoreEventState(true);
            }

            if (!paxisbutton->getIgnoreEventState())
            {
                paxisbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i = 0; i < currentset->getNumberHats(); i++)
    {
        JoyDPad *dpad = currentset->getJoyDPad(i);
        QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
        QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
        while (iter.hasNext())
        {
            JoyDPadButton *dpadbutton = iter.next().value();

            if (dpad->getJoyMode() != JoyDPad::EightWayMode)
            {
                if ((dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown))
                {
                    connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                        showDPadButtonDialog(dpadbutton);
                    });
                }
            }
            else
            {
                connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                    showDPadButtonDialog(dpadbutton);
                });
            }

            if (!dpadbutton->getIgnoreEventState())
            {
                dpadbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i=0; i < currentset->getNumberVDPads(); i++)
    {
        VDPad *dpad = currentset->getVDPad(i);
        if (dpad != nullptr)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
            while (iter.hasNext())
            {
                JoyDPadButton *dpadbutton = iter.next().value();

                if (dpad->getJoyMode() != JoyDPad::EightWayMode)
                {
                    if ((dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown))
                    {
                        connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                            showDPadButtonDialog(dpadbutton);
                        });
                    }
                }
                else
                {
                    connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                        showDPadButtonDialog(dpadbutton);
                    });
                }

                if (!dpadbutton->getIgnoreEventState())
                {
                    dpadbutton->setIgnoreEventState(true);
                }
            }
        }
    }

    for (int i = 0; i < currentset->getNumberButtons(); i++)
    {
        JoyButton *button = currentset->getJoyButton(i);
        if ((button != nullptr) && !button->isPartVDPad())
        {
            connect(button, &JoyButton::clicked, this, [this, button] {
                showButtonDialog(button);
            });

            if (!button->getIgnoreEventState())
            {
                button->setIgnoreEventState(true);
            }
        }
    }

    connect(this, &QuickSetDialog::finished, this, &QuickSetDialog::restoreButtonStates);
}


QuickSetDialog::QuickSetDialog(InputDevice *joystick, ButtonEditDialogHelper* helper, const char* invokeString,
                               int code, int alias, int index,
                               JoyButtonSlot::JoySlotInputAction mode,
                               bool withClear, bool withTrue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetDialog)
{
    ui->setupUi(this);

    show();

    this->joystick = joystick;
    this->invokeString = invokeString;
    this->code = code;
    this->alias = alias;
    this->index = index;
    this->mode = mode;
    this->withClear = withClear;
    this->withTrue = withTrue;
    this->helper = helper;
    lastButton = nullptr;

    setWindowTitle(trUtf8("Quick Set %1").arg(joystick->getName()));

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);

    SetJoystick *currentset = joystick->getActiveSetJoystick();

    QString temp = ui->joystickDialogLabel->text();
    temp = temp.arg(joystick->getSDLName()).arg(joystick->getName());
    ui->joystickDialogLabel->setText(temp);

    for (int i=0; i < currentset->getNumberSticks(); i++)
    {
        JoyControlStick *stick = currentset->getJoyStick(i);
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stickButtons);
        while (iter.hasNext())
        {
            JoyControlStickButton *stickbutton = iter.next().value();

            if (stick->getJoyMode() != JoyControlStick::EightWayMode)
            {
                if ((stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickLeftUp)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickRightUp)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickLeftDown)) &&
                    (stickbutton->getJoyNumber() != static_cast<int>(JoyControlStick::StickRightDown)))
                {
                    connect(stickbutton, &JoyControlStickButton::clicked, this, [this, stickbutton]() {
                        showStickButtonDialog(stickbutton);
                    });
                }
            }
            else
            {
                connect(stickbutton, &JoyControlStickButton::clicked, this, [this, stickbutton]() {
                    showStickButtonDialog(stickbutton);
                });
            }

            if (!stickbutton->getIgnoreEventState())
            {
                stickbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i = 0; i < currentset->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            JoyAxisButton *paxisbutton = axis->getPAxisButton();

            connect(naxisbutton, &JoyAxisButton::clicked, this, [this, naxisbutton]() {
                showAxisButtonDialog(naxisbutton);
            });

            connect(paxisbutton, &JoyAxisButton::clicked, this, [this, paxisbutton]() {
                showAxisButtonDialog(paxisbutton);
            });

            if (!naxisbutton->getIgnoreEventState())
            {
                naxisbutton->setIgnoreEventState(true);
            }

            if (!paxisbutton->getIgnoreEventState())
            {
                paxisbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i=0; i < currentset->getNumberHats(); i++)
    {
        JoyDPad *dpad = currentset->getJoyDPad(i);
        QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
        QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
        while (iter.hasNext())
        {
            JoyDPadButton *dpadbutton = iter.next().value();

            if (dpad->getJoyMode() != JoyDPad::EightWayMode)
            {
                if ((dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown) &&
                    (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown))
                {
                    connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                        showDPadButtonDialog(dpadbutton);
                    });
                }
            }
            else
            {
                connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                    showDPadButtonDialog(dpadbutton);
                });
            }

            if (!dpadbutton->getIgnoreEventState())
            {
                dpadbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i=0; i < currentset->getNumberVDPads(); i++)
    {
        VDPad *dpad = currentset->getVDPad(i);
        if (dpad != nullptr)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
            while (iter.hasNext())
            {
                JoyDPadButton *dpadbutton = iter.next().value();

                if (dpad->getJoyMode() != JoyDPad::EightWayMode)
                {
                    if ((dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown) &&
                        (dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown))
                    {
                        connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                            showDPadButtonDialog(dpadbutton);
                        });
                    }
                }
                else
                {
                    connect(dpadbutton, &JoyDPadButton::clicked, this, [this, dpadbutton] {
                        showDPadButtonDialog(dpadbutton);
                    });
                }

                if (!dpadbutton->getIgnoreEventState())
                {
                    dpadbutton->setIgnoreEventState(true);
                }
            }
        }
    }

    for (int i = 0; i < currentset->getNumberButtons(); i++)
    {
        JoyButton *button = currentset->getJoyButton(i);
        if ((button != nullptr) && !button->isPartVDPad())
        {
            connect(button, &JoyButton::clicked, this, [this, button] {
                showButtonDialog(button);
            });

            if (!button->getIgnoreEventState())
            {
                button->setIgnoreEventState(true);
            }
        }
    }

    connect(this, &QuickSetDialog::finished, this, &QuickSetDialog::restoreButtonStates);


}

QuickSetDialog::~QuickSetDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void QuickSetDialog::showAxisButtonDialog(JoyAxisButton* joybtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastButton = joybtn;

    if (helper != nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in QuickSetDialog";
    #endif

    if (withClear) {

        QMetaObject::invokeMethod(lastButton, "clearSlotsEventReset", Q_ARG(bool, withTrue));

    }


    // when alias exists but not index
    if ((alias != -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when alias doesn't exists and index too
    } else if ((alias == -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when all exist (code, alias, index)
    } else {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

    }

    this->close();
}

void QuickSetDialog::showButtonDialog(JoyButton* joybtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastButton = joybtn; // static_cast

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());


    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in QuickSetDialog";
    #endif


    if (withClear) {

        QMetaObject::invokeMethod(lastButton, "clearSlotsEventReset", Q_ARG(bool, withTrue));

    }



    // when alias exists but not index
    if ((alias != -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when alias doesn't exists and index too
    } else if ((alias == -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when all exist (code, alias, index)
    } else {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

    }


    this->close();
}

void QuickSetDialog::showStickButtonDialog(JoyControlStickButton* joyctrlstickbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastButton = joyctrlstickbtn;

    // sometimes appears situation, when we want to map an axis of stick,
    // it's detected some state between pressing stick button and moving axis.
    // It chooses a function for pressing a stick, but later appears problem,
    // because correct and prepared value is from the axis one, so static_cast
    // for stick button is failing
    // App can sometimes ask to three times for a button, but it never is going to crash now
    // if lastButton is still null pointer, check, if correct value comes from axis
    if (lastButton == nullptr) lastButton = qobject_cast<JoyAxisButton*>(sender());

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    // however when it's still a problem
    // skip it and try again soon
    // should be ok then
    if (lastButton != nullptr) {

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in QuickSetDialog";
    #endif

    if (withClear) {

        QMetaObject::invokeMethod(lastButton, "clearSlotsEventReset", Q_ARG(bool, withTrue));

    }

    // when alias exists but not index
    if ((alias != -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when alias doesn't exists and index too
    } else if ((alias == -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when all exist (code, alias, index)
    } else {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

    }

    }

    this->close();
}


void QuickSetDialog::showDPadButtonDialog(JoyDPadButton* joydpadbtn)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastButton = joydpadbtn;

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());


    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in QuickSetDialog";
    #endif

    if (withClear) {

        QMetaObject::invokeMethod(lastButton, "clearSlotsEventReset", Q_ARG(bool, withTrue));

    }

    // when alias exists but not index
    if ((alias != -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when alias doesn't exists and index too
    } else if ((alias == -1) && (index == -1)) {

        QMetaObject::invokeMethod(helper, invokeString, Qt::QueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

     // when all exist (code, alias, index)
    } else {

        QMetaObject::invokeMethod(helper, invokeString, Qt::BlockingQueuedConnection,
                                  Q_ARG(int, code),
                                  Q_ARG(int, alias),
                                  Q_ARG(int, index),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, mode));

    }


    this->close();
}


void QuickSetDialog::restoreButtonStates()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentset = joystick->getActiveSetJoystick();

    for (int i = 0; i < currentset->getNumberSticks(); i++)
    {
        JoyControlStick *stick = currentset->getJoyStick(i);
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stickButtons);
        while (iter.hasNext())
        {
            JoyControlStickButton *stickbutton = iter.next().value();
            if (stickbutton->getIgnoreEventState())
            {
                stickbutton->setIgnoreEventState(false);
            }

            disconnect(stickbutton, &JoyControlStickButton::clicked, this, nullptr);
        }
    }

    for (int i=0; i < currentset->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            if (naxisbutton->getIgnoreEventState())
            {
                naxisbutton->setIgnoreEventState(false);
            }
            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            if (paxisbutton->getIgnoreEventState())
            {
                paxisbutton->setIgnoreEventState(false);
            }
            disconnect(naxisbutton, &JoyAxisButton::clicked, this, nullptr);
            disconnect(paxisbutton, &JoyAxisButton::clicked, this, nullptr);
        }
    }

    for (int i=0; i < currentset->getNumberHats(); i++)
    {
        JoyDPad *dpad = currentset->getJoyDPad(i);
        QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
        QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
        while (iter.hasNext())
        {
            JoyDPadButton *dpadbutton = iter.next().value();
            if (dpadbutton->getIgnoreEventState())
            {
                dpadbutton->setIgnoreEventState(false);
            }
            disconnect(dpadbutton, &JoyDPadButton::clicked, this, nullptr);
        }
    }

    for (int i=0; i < currentset->getNumberVDPads(); i++)
    {
        VDPad *dpad = currentset->getVDPad(i);
        if (dpad != nullptr)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
            while (iter.hasNext())
            {
                JoyDPadButton *dpadbutton = iter.next().value();
                if (dpadbutton->getIgnoreEventState())
                {
                    dpadbutton->setIgnoreEventState(false);
                }
                disconnect(dpadbutton, &JoyDPadButton::clicked, this, nullptr);
            }
        }
    }

    for (int i=0; i < currentset->getNumberButtons(); i++)
    {
        JoyButton *button = currentset->getJoyButton(i);
        if ((button != nullptr) && !button->isPartVDPad())
        {
            if (button->getIgnoreEventState())
            {
                button->setIgnoreEventState(false);
            }
            disconnect(button, &JoyButton::clicked, this, nullptr);
        }
    }

    currentset->release();
}


JoyButton* QuickSetDialog::getLastPressedButton() const {

    return lastButton;
}

InputDevice *QuickSetDialog::getJoystick() const {

    return joystick;
}

QDialog *QuickSetDialog::getCurrentButtonDialog() const {

    return currentButtonDialog;
}

const char* QuickSetDialog::getInvokeString() const {

    return invokeString;
}

ButtonEditDialogHelper* QuickSetDialog::getHelper() const {

    return helper;
}

JoyButtonSlot::JoySlotInputAction QuickSetDialog::getMode() const {

    return mode;
}
