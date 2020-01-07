/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "joydpadbutton.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "joydpad.h"
#include "setjoystick.h"
#include "event.h"

#include <QDebug>


// Initially, qualify direction as the button's index
JoyDPadButton::JoyDPadButton(int direction, int originset, JoyDPad* dpad, SetJoystick *parentSet, QObject *parent) :
    JoyButton(direction, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_direction = direction;
    m_dpad = dpad;
}


QString JoyDPadButton::getDirectionName() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString ();

    switch(m_direction)
    {
        case 1:
            label.append(tr("Up"));
        break;

        case 2:
            label.append(tr("Right"));
        break;

        case 3:
            label.append(tr("Up")).append("+").append(tr("Right"));
        break;

        case 4:
            label.append(tr("Down"));
        break;

        case 6:
            label.append(tr("Down")).append("+").append(tr("Right"));
        break;

        case 8:
            label.append(tr("Left"));
        break;

        case 9:
            label.append(tr("Up")).append("+").append(tr("Left"));
        break;

        case 12:
            label.append(tr("Down")).append("+").append(tr("Left"));
        break;
    }

    return label;
}


QString JoyDPadButton::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyDPadButton::xmlName;
}


int JoyDPadButton::getRealJoyNumber() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index;
}


QString JoyDPadButton::getPartialName(bool forceFullFormat, bool displayNames) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = m_dpad->getName().append(" - ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }

        temp.append(defaultButtonName);
    }
    else
    {
        temp.append(tr("Button")).append(" ");
        temp.append(getDirectionName());
    }

    return temp;
}


void JoyDPadButton::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::reset();
}


void JoyDPadButton::reset(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(index);
    reset();
}


void JoyDPadButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    Q_UNUSED(updateActiveString);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(m_index, m_dpad->getJoyNumber(), setSelection, condition);
        }
        else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(m_index, m_dpad->getJoyNumber(), setSelection, SetChangeDisabled);
        }

        setSelectionCondition = condition;
    }
    else if (passive)
    {
        setSelectionCondition = condition;
    }

    if (setSelectionCondition == SetChangeDisabled)
    {
        setChangeSetSelection(-1);
    }

    if (setSelectionCondition != oldCondition)
    {
        buildActiveZoneSummaryString();
        emit propertyUpdated();
    }
}


JoyDPad* JoyDPadButton::getDPad() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_dpad;
}


int JoyDPadButton::getDirection() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_direction;
}
