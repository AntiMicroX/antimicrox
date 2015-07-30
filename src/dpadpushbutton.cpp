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

#include "dpadpushbutton.h"
#include "dpadcontextmenu.h"

DPadPushButton::DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->dpad = dpad;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

JoyDPad* DPadPushButton::getDPad()
{
    return dpad;
}

QString DPadPushButton::generateLabel()
{
    QString temp;
    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName(false, displayNames));
    }
    else
    {
        temp.append(dpad->getName());
    }

    return temp;
}

void DPadPushButton::disableFlashes()
{
    disconnect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    connect(dpad, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(dpad, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

void DPadPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    DPadContextMenu *contextMenu = new DPadContextMenu(dpad, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void DPadPushButton::tryFlash()
{
    if (dpad->getCurrentDirection() != static_cast<int>(JoyDPadButton::DpadCentered))
    {
        flash();
    }
}
