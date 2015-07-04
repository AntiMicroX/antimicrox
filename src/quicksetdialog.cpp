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

#include <QHash>
#include <QHashIterator>

#include "quicksetdialog.h"
#include "ui_quicksetdialog.h"

#include "setjoystick.h"
#include "buttoneditdialog.h"

QuickSetDialog::QuickSetDialog(InputDevice *joystick, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;
    this->currentButtonDialog = 0;

    setWindowTitle(tr("Quick Set %1").arg(joystick->getName()));

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
                if (stickbutton->getJoyNumber() != JoyControlStick::StickLeftUp &&
                    stickbutton->getJoyNumber() != JoyControlStick::StickRightUp &&
                    stickbutton->getJoyNumber() != JoyControlStick::StickLeftDown &&
                    stickbutton->getJoyNumber() != JoyControlStick::StickRightDown)
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
                if (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp &&
                    dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp &&
                    dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown &&
                    dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown)
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
        if (dpad)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*dpadbuttons);
            while (iter.hasNext())
            {
                JoyDPadButton *dpadbutton = iter.next().value();

                if (dpad->getJoyMode() != JoyDPad::EightWayMode)
                {
                    if (dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftUp &&
                        dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightUp &&
                        dpadbutton->getJoyNumber() != JoyDPadButton::DpadLeftDown &&
                        dpadbutton->getJoyNumber() != JoyDPadButton::DpadRightDown)
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
        if (button && !button->isPartVDPad())
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
    delete ui;
}

void QuickSetDialog::showAxisButtonDialog()
{
    if (!currentButtonDialog)
    {
        JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(sender());
        currentButtonDialog = new ButtonEditDialog(axisbutton, this);
        currentButtonDialog->show();
        connect(currentButtonDialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showButtonDialog()
{
    if (!currentButtonDialog)
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        currentButtonDialog = new ButtonEditDialog(button, this);
        currentButtonDialog->show();
        connect(currentButtonDialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showStickButtonDialog()
{
    if (!currentButtonDialog)
    {
        JoyControlStickButton *stickbutton = static_cast<JoyControlStickButton*>(sender());
        currentButtonDialog = new ButtonEditDialog(stickbutton, this);
        currentButtonDialog->show();
        connect(currentButtonDialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showDPadButtonDialog()
{
    if (!currentButtonDialog)
    {
        JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());
        currentButtonDialog = new ButtonEditDialog(dpadbutton, this);
        currentButtonDialog->show();
        connect(currentButtonDialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::nullifyDialogPointer()
{
    if (currentButtonDialog)
    {
        currentButtonDialog = 0;
        emit buttonDialogClosed();
    }
}

void QuickSetDialog::restoreButtonStates()
{
    SetJoystick *currentset = joystick->getActiveSetJoystick();

    for (int i=0; i < currentset->getNumberSticks(); i++)
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
        if (dpad)
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
        if (button && !button->isPartVDPad())
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
