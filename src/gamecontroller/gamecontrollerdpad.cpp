/* antimicroX Gamepad to KB+M event mapper
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

#include "gamecontrollerdpad.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "setjoystick.h"
#include "joybutton.h"
#include "xml/joydpadxml.h"

#include <QDebug>


GameControllerDPad::GameControllerDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                                       int index, int originset, SetJoystick *parentSet, QObject *parent) :
    VDPad(upButton, downButton, leftButton, rightButton, index, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


QString GameControllerDPad::getName(bool forceFullFormat, bool displayName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString();

    if (!getDpadName().isEmpty() && displayName)
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(getDpadName());
    }
    else if (!getDefaultDpadName().isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(getDefaultDpadName());
    }
    else
    {
        label.append(tr("DPad")).append(" ");
        label.append(QString::number(getRealJoyNumber()));
    }

    return label;
}


QString GameControllerDPad::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameControllerDPad::xmlName;
}
