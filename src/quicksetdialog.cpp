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

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
                if ((stickbutton->getJoyNumber() != JoyControlStick::StickLeftUp) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickRightUp) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickLeftDown) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickRightDown))
                {
                    connect(stickbutton, SIGNAL(clicked(int)), this, SLOT(showStickButtonDialog()));
                }
            }
            else
            {
                connect(stickbutton, SIGNAL(clicked(int)), this, SLOT(showStickButtonDialog()));
            }

            if (!stickbutton->getIgnoreEventState())
            {
                stickbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i=0; i < currentset->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            JoyAxisButton *paxisbutton = axis->getPAxisButton();

            connect(naxisbutton, SIGNAL(clicked(int)), this, SLOT(showAxisButtonDialog()));
            connect(paxisbutton, SIGNAL(clicked(int)), this, SLOT(showAxisButtonDialog()));

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
                    connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                }
            }
            else
            {
                connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
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
                        connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                    }
                }
                else
                {
                    connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                }

                if (!dpadbutton->getIgnoreEventState())
                {
                    dpadbutton->setIgnoreEventState(true);
                }
            }
        }
    }

    for (int i=0; i < currentset->getNumberButtons(); i++)
    {
        JoyButton *button = currentset->getJoyButton(i);
        if ((button != nullptr) && !button->isPartVDPad())
        {
            connect(button, SIGNAL(clicked(int)), this, SLOT(showButtonDialog()));
            if (!button->getIgnoreEventState())
            {
                button->setIgnoreEventState(true);
            }
        }
    }

    connect(this, SIGNAL(finished(int)), this, SLOT(restoreButtonStates()));
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
   // currentset->release();
   // joystick->resetButtonDownCount();

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
                if ((stickbutton->getJoyNumber() != JoyControlStick::StickLeftUp) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickRightUp) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickLeftDown) &&
                    (stickbutton->getJoyNumber() != JoyControlStick::StickRightDown))
                {
                    connect(stickbutton, SIGNAL(clicked(int)), this, SLOT(showStickButtonDialog()));
                }
            }
            else
            {
                connect(stickbutton, SIGNAL(clicked(int)), this, SLOT(showStickButtonDialog()));
            }

            if (!stickbutton->getIgnoreEventState())
            {
                stickbutton->setIgnoreEventState(true);
            }
        }
    }

    for (int i=0; i < currentset->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

        if (!axis->isPartControlStick() && axis->hasControlOfButtons())
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            JoyAxisButton *paxisbutton = axis->getPAxisButton();

            connect(naxisbutton, SIGNAL(clicked(int)), this, SLOT(showAxisButtonDialog()));
            connect(paxisbutton, SIGNAL(clicked(int)), this, SLOT(showAxisButtonDialog()));

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
                    connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                }
            }
            else
            {
                connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
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
                        connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                    }
                }
                else
                {
                    connect(dpadbutton, SIGNAL(clicked(int)), this, SLOT(showDPadButtonDialog()));
                }

                if (!dpadbutton->getIgnoreEventState())
                {
                    dpadbutton->setIgnoreEventState(true);
                }
            }
        }
    }

    for (int i=0; i < currentset->getNumberButtons(); i++)
    {
        JoyButton *button = currentset->getJoyButton(i);
        if ((button != nullptr) && !button->isPartVDPad())
        {
            connect(button, SIGNAL(clicked(int)), this, SLOT(showButtonDialog()));
            if (!button->getIgnoreEventState())
            {
                button->setIgnoreEventState(true);
            }
        }
    }

    connect(this, SIGNAL(finished(int)), this, SLOT(restoreButtonStates()));


}

QuickSetDialog::~QuickSetDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    delete ui;
}

void QuickSetDialog::showAxisButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    lastButton = qobject_cast<JoyAxisButton*>(sender());

    if (helper != nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());

    qDebug() << "Thread in QuickSetDialog";

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

void QuickSetDialog::showButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    lastButton = qobject_cast<JoyButton*>(sender()); // static_cast

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());


    qDebug() << "Thread in QuickSetDialog";


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

void QuickSetDialog::showStickButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    lastButton = qobject_cast<JoyControlStickButton*>(sender());

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());

    qDebug() << "Thread in QuickSetDialog";

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


void QuickSetDialog::showDPadButtonDialog()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    lastButton = qobject_cast<JoyDPadButton*>(sender());

    if (helper == nullptr) {
        helper = new ButtonEditDialogHelper();
    }

    helper->setThisButton(lastButton);

    helper->moveToThread(lastButton->thread());


    qDebug() << "Thread in QuickSetDialog";

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

            disconnect(stickbutton, SIGNAL(clicked(int)), this, 0);
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
            disconnect(naxisbutton, SIGNAL(clicked(int)), this, 0);
            disconnect(paxisbutton, SIGNAL(clicked(int)), this, 0);
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
            disconnect(dpadbutton, SIGNAL(clicked(int)), this, 0);
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
                disconnect(dpadbutton, SIGNAL(clicked(int)), this, 0);
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
            disconnect(button, SIGNAL(clicked(int)), this, 0);
        }
    }

    currentset->release();
}


JoyButton* QuickSetDialog::getLastPressedButton() {

    return lastButton;
}
