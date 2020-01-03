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

#include "dpadpushbutton.h"

#include "messagehandler.h"
#include "joydpad.h"
#include "dpadcontextmenu.h"

#include <QDebug>

DPadPushButton::DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->dpad = dpad;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &DPadPushButton::customContextMenuRequested, this, &DPadPushButton::showContextMenu);

    connect(dpad, &JoyDPad::dpadNameChanged, this, &DPadPushButton::refreshLabel);
}

JoyDPad* DPadPushButton::getDPad() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return dpad;
}

QString DPadPushButton::generateLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (!dpad->getDpadName().isEmpty()) temp.append(dpad->getName(false, ifDisplayNames()));
    else temp.append(dpad->getName());

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Dpad button name is: " << temp;
    #endif

    return temp;
}

void DPadPushButton::disableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(dpad, &JoyDPad::active, this, &DPadPushButton::flash);
    disconnect(dpad, &JoyDPad::released, this, &DPadPushButton::unflash);

    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(dpad, &JoyDPad::active, this, &DPadPushButton::flash, Qt::QueuedConnection);
    connect(dpad, &JoyDPad::released, this, &DPadPushButton::unflash, Qt::QueuedConnection);
}

void DPadPushButton::showContextMenu(const QPoint &point)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPoint globalPos = this->mapToGlobal(point);
    DPadContextMenu *contextMenu = new DPadContextMenu(dpad, this);

    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void DPadPushButton::tryFlash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (dpad->getCurrentDirection() != static_cast<int>(JoyDPadButton::DpadCentered))
    {
        flash();
    }
}
