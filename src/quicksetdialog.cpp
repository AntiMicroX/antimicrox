#include <QHash>
#include <QHashIterator>

#include "quicksetdialog.h"
#include "ui_quicksetdialog.h"

#include "setjoystick.h"
#include "buttoneditdialog.h"

QuickSetDialog::QuickSetDialog(Joystick *joystick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::QuickSetDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;

    setWindowTitle(tr("Quick Set Joystick %1").arg(joystick->getRealJoyNumber()));

    SetJoystick *currentset = joystick->getActiveSetJoystick();

    QString temp = ui->joystickDialogLabel->text();
    temp = temp.arg(joystick->getRealJoyNumber());
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
    if (!dialog)
    {
        JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(sender());
        dialog = new ButtonEditDialog(axisbutton, this);
        dialog->show();
        connect(dialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showButtonDialog()
{
    if (!dialog)
    {
        JoyButton *button = static_cast<JoyButton*>(sender());
        dialog = new ButtonEditDialog(button, this);
        dialog->show();
        connect(dialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showStickButtonDialog()
{
    if (!dialog)
    {
        JoyControlStickButton *stickbutton = static_cast<JoyControlStickButton*>(sender());
        dialog = new ButtonEditDialog(stickbutton, this);
        dialog->show();
        connect(dialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::showDPadButtonDialog()
{
    if (!dialog)
    {
        JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());
        dialog = new ButtonEditDialog(dpadbutton, this);
        dialog->show();
        connect(dialog, SIGNAL(finished(int)), this, SLOT(nullifyDialogPointer()));
    }
}

void QuickSetDialog::nullifyDialogPointer()
{
    if (dialog)
    {
        dialog = 0;
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
}
