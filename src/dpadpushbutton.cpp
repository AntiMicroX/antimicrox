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
#include "joydpad.h"
#include "dpadcontextmenu.h"

#include <QDebug>

DPadPushButton::DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->dpad = dpad;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

JoyDPad* DPadPushButton::getDPad() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return dpad;
}

QString DPadPushButton::generateLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName(false, ifDisplayNames()));
    }
    else
    {
        temp.append(dpad->getName());
    }

    qDebug() << "Dpad button name is: " << temp;
    return temp;
}

void DPadPushButton::disableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(dpad, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(dpad, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

void DPadPushButton::showContextMenu(const QPoint &point)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QPoint globalPos = this->mapToGlobal(point);
    DPadContextMenu *contextMenu = new DPadContextMenu(dpad, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void DPadPushButton::tryFlash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (dpad->getCurrentDirection() != static_cast<int>(JoyDPadButton::DpadCentered))
    {
        flash();
    }
}
