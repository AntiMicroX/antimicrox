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

#include "joyaxiswidget.h"

#include "messagehandler.h"
#include "joyaxiscontextmenu.h"
#include "joyaxis.h"

#include <QDebug>

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->axis = axis;

    refreshLabel();
    enableFlashes();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoyAxisWidget::customContextMenuRequested, this, &JoyAxisWidget::showContextMenu);

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    tryFlash();

    connect(axis, &JoyAxis::throttleChanged, this, &JoyAxisWidget::refreshLabel);
    connect(axis, &JoyAxis::axisNameChanged, this, &JoyAxisWidget::refreshLabel);
    connect(nAxisButton, &JoyAxisButton::propertyUpdated, this, &JoyAxisWidget::refreshLabel);
    connect(pAxisButton, &JoyAxisButton::propertyUpdated, this, &JoyAxisWidget::refreshLabel);
    connect(nAxisButton, &JoyAxisButton::activeZoneChanged, this, &JoyAxisWidget::refreshLabel);
    connect(pAxisButton, &JoyAxisButton::activeZoneChanged, this, &JoyAxisWidget::refreshLabel);

    axis->establishPropertyUpdatedConnection();
    nAxisButton->establishPropertyUpdatedConnections();
    pAxisButton->establishPropertyUpdatedConnections();
}

JoyAxis* JoyAxisWidget::getAxis() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return axis;
}

void JoyAxisWidget::disableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(axis, &JoyAxis::active, this, &JoyAxisWidget::flash);
    disconnect(axis, &JoyAxis::released, this, &JoyAxisWidget::unflash);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(axis, &JoyAxis::active, this, &JoyAxisWidget::flash, Qt::QueuedConnection);
    connect(axis, &JoyAxis::released, this, &JoyAxisWidget::unflash, Qt::QueuedConnection);
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyAxisWidget::generateLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    temp = axis->getName(false, ifDisplayNames()).replace("&", "&&");

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Name of joy axis is: " << temp;
    #endif

    return temp;
}

void JoyAxisWidget::showContextMenu(const QPoint &point)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPoint globalPos = this->mapToGlobal(point);
    JoyAxisContextMenu *contextMenu = new JoyAxisContextMenu(axis, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyAxisWidget::tryFlash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    if (nAxisButton->getButtonState() || pAxisButton->getButtonState())
    {
        flash();
    }
}
