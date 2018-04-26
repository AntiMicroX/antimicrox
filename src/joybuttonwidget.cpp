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

#include "joybuttonwidget.h"
#include "joybuttoncontextmenu.h"
#include "joybutton.h"

#include <QMenu>
#include <QDebug>

JoyButtonWidget::JoyButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->button = button;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(button, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));
}

JoyButton* JoyButtonWidget::getJoyButton() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return button;
}

void JoyButtonWidget::disableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

QString JoyButtonWidget::generateLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    temp = button->getName(false, ifDisplayNames()).replace("&", "&&");

    qDebug() << "Name of joy button is: " << temp;
    return temp;
}

void JoyButtonWidget::showContextMenu(const QPoint &point)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyButtonWidget::tryFlash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (button->getButtonState())
    {
        flash();
    }
}
