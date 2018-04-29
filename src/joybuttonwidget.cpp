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

#include "messagehandler.h"
#include "joybuttoncontextmenu.h"
#include "joybutton.h"

#include <QMenu>
#include <QDebug>

JoyButtonWidget::JoyButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return button;
}

void JoyButtonWidget::disableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

QString JoyButtonWidget::generateLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    temp = button->getName(false, ifDisplayNames()).replace("&", "&&");

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Name of joy button is: " << temp;
    #endif

    return temp;
}

void JoyButtonWidget::showContextMenu(const QPoint &point)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyButtonWidget::tryFlash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (button->getButtonState())
    {
        flash();
    }
}
