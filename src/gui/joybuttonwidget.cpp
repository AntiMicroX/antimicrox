/* antimicrox Gamepad to KB+M event mapper
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

#include "joybuttonwidget.h"

#include "joybuttoncontextmenu.h"
#include "joybuttontypes/joybutton.h"

#include <QDebug>
#include <QMenu>
#include <QPoint>

JoyButtonWidget::JoyButtonWidget(JoyButton *button, bool displayNames, QWidget *parent)
    : FlashButtonWidget(displayNames, parent)
{
    m_button = button;

    refreshLabel();
    enableFlashes();

    tryFlash();

    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &JoyButtonWidget::customContextMenuRequested, this, &JoyButtonWidget::showContextMenu);
    connect(button, &JoyButton::propertyUpdated, this, &JoyButtonWidget::refreshLabel);
    connect(button, &JoyButton::activeZoneChanged, this, &JoyButtonWidget::refreshLabel);
}

JoyButton *JoyButtonWidget::getJoyButton() const { return m_button; }

void JoyButtonWidget::disableFlashes()
{
    disconnect(m_button, &JoyButton::clicked, this, &JoyButtonWidget::flash);
    disconnect(m_button, &JoyButton::released, this, &JoyButtonWidget::unflash);

    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    connect(m_button, &JoyButton::clicked, this, &JoyButtonWidget::flash, Qt::QueuedConnection);
    connect(m_button, &JoyButton::released, this, &JoyButtonWidget::unflash, Qt::QueuedConnection);
}

QString JoyButtonWidget::generateLabel()
{
    QString temp = m_button->getName(false, ifDisplayNames()).replace("&", "&&");

    qDebug() << "Name of joy button is: " << temp;

    return temp;
}

void JoyButtonWidget::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(m_button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyButtonWidget::tryFlash()
{
    if (m_button->getButtonState())
        flash();
}
