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

#include "vdpad.h"

const QString VDPad::xmlName = "vdpad";

VDPad::VDPad(int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyDPad(index, originset, parentSet, parent)
{
    this->upButton = 0;
    this->downButton = 0;
    this->leftButton = 0;
    this->rightButton = 0;

    pendingVDPadEvent = false;
}

VDPad::VDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
             int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyDPad(index, originset, parentSet, parent)
{
    this->upButton = upButton;
    upButton->setVDPad(this);

    this->downButton = downButton;
    downButton->setVDPad(this);

    this->leftButton = leftButton;
    leftButton->setVDPad(this);

    this->rightButton = rightButton;
    rightButton->setVDPad(this);

    pendingVDPadEvent = false;
}

VDPad::~VDPad()
{
    if (upButton)
    {
        upButton->removeVDPad();
        upButton = 0;
    }

    if (downButton)
    {
        downButton->removeVDPad();
        downButton = 0;
    }

    if (leftButton)
    {
        leftButton->removeVDPad();
        leftButton = 0;
    }

    if (rightButton)
    {
        rightButton->removeVDPad();
        rightButton = 0;
    }
}

QString VDPad::getXmlName()
{
    return this->xmlName;
}

QString VDPad::getName(bool forceFullFormat, bool displayName)
{
    QString label;

    if (!dpadName.isEmpty() && displayName)
    {
        if (forceFullFormat)
        {
            label.append(tr("VDPad")).append(" ");
        }

        label.append(dpadName);
    }
    else if (!defaultDPadName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("VDPad")).append(" ");
        }
        label.append(defaultDPadName);
    }
    else
    {
        label.append(tr("VDPad")).append(" ");
        label.append(QString::number(getRealJoyNumber()));
    }

    return label;
}

void VDPad::joyEvent(bool pressed, bool ignoresets)
{
    Q_UNUSED(pressed);

    int tempDirection = static_cast<int>(JoyDPadButton::DpadCentered);

    /*
     * Check which buttons are currently active
     */
    if (upButton && upButton->getButtonState())
    {
        tempDirection |= JoyDPadButton::DpadUp;
    }

    if (downButton && downButton->getButtonState())
    {
        tempDirection |= JoyDPadButton::DpadDown;
    }

    if (leftButton && leftButton->getButtonState())
    {
        tempDirection |= JoyDPadButton::DpadLeft;
    }

    if (rightButton && rightButton->getButtonState())
    {
        tempDirection |= JoyDPadButton::DpadRight;
    }

    JoyDPad::joyEvent(tempDirection, ignoresets);

    pendingVDPadEvent = false;
}

void VDPad::addVButton(JoyDPadButton::JoyDPadDirections direction, JoyButton *button)
{
    if (direction == JoyDPadButton::DpadUp)
    {
        if (upButton)
        {
            upButton->removeVDPad();
        }
        upButton = button;
        upButton->setVDPad(this);
    }
    else if (direction == JoyDPadButton::DpadDown)
    {
        if (downButton)
        {
            downButton->removeVDPad();
        }
        downButton = button;
        downButton->setVDPad(this);
    }
    else if (direction == JoyDPadButton::DpadLeft)
    {
        if (leftButton)
        {
            leftButton->removeVDPad();
        }
        leftButton = button;
        leftButton->setVDPad(this);
    }
    else if (direction == JoyDPadButton::DpadRight)
    {
        if (rightButton)
        {
            rightButton->removeVDPad();
        }
        rightButton = button;
        rightButton->setVDPad(this);
    }
}

void VDPad::removeVButton(JoyDPadButton::JoyDPadDirections direction)
{
    if (direction == JoyDPadButton::DpadUp && upButton)
    {
        upButton->removeVDPad();
        upButton = 0;
    }
    else if (direction == JoyDPadButton::DpadDown && downButton)
    {
        downButton->removeVDPad();
        downButton = 0;
    }
    else if (direction == JoyDPadButton::DpadLeft && leftButton)
    {
        leftButton->removeVDPad();
        leftButton = 0;
    }
    else if (direction == JoyDPadButton::DpadRight && rightButton)
    {
        rightButton->removeVDPad();
        rightButton = 0;
    }
}

void VDPad::removeVButton(JoyButton *button)
{
    if (button && button == upButton)
    {
        upButton->removeVDPad();
        upButton = 0;
    }
    else if (button && button == downButton)
    {
        downButton->removeVDPad();
        downButton = 0;
    }
    else if (button && button == leftButton)
    {
        leftButton->removeVDPad();
        leftButton = 0;
    }
    else if (button && button == rightButton)
    {
        rightButton->removeVDPad();
        rightButton = 0;
    }
}

bool VDPad::isEmpty()
{
    bool empty = true;

    if (upButton || downButton || leftButton || rightButton)
    {
        empty = false;
    }

    return empty;
}

JoyButton* VDPad::getVButton(JoyDPadButton::JoyDPadDirections direction)
{
    JoyButton *button = 0;
    if (direction == JoyDPadButton::DpadUp)
    {
        button = upButton;
    }
    else if (direction == JoyDPadButton::DpadDown)
    {
        button = downButton;
    }
    else if (direction == JoyDPadButton::DpadLeft)
    {
        button = leftButton;
    }
    else if (direction == JoyDPadButton::DpadRight)
    {
        button = rightButton;
    }

    return button;
}

bool VDPad::hasPendingEvent()
{
    return pendingVDPadEvent;
}

void VDPad::queueJoyEvent(bool ignoresets)
{
    Q_UNUSED(ignoresets);

    pendingVDPadEvent = true;
}

void VDPad::activatePendingEvent()
{
    if (pendingVDPadEvent)
    {
        // Always use true. The proper direction value will be determined
        // in the joyEvent method.
        joyEvent(true);

        pendingVDPadEvent = false;
    }

}

void VDPad::clearPendingEvent()
{
    pendingVDPadEvent = false;
}
