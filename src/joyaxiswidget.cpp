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

#include "joyaxiswidget.h"

#include "joyaxis.h"
#include "joyaxiscontextmenu.h"
#include "messagehandler.h"

#include <QDebug>

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent)
    : FlashButtonWidget(displayNames, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_axis = axis;

    refreshLabel();
    enableFlashes();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoyAxisWidget::customContextMenuRequested, this, &JoyAxisWidget::showContextMenu);

    JoyAxisButton *nAxisButton = m_axis->getNAxisButton();
    JoyAxisButton *pAxisButton = m_axis->getPAxisButton();

    tryFlash();

    connect(m_axis, &JoyAxis::throttleChanged, this, &JoyAxisWidget::refreshLabel);
    connect(m_axis, &JoyAxis::axisNameChanged, this, &JoyAxisWidget::refreshLabel);
    connect(nAxisButton, &JoyAxisButton::propertyUpdated, this, &JoyAxisWidget::refreshLabel);
    connect(pAxisButton, &JoyAxisButton::propertyUpdated, this, &JoyAxisWidget::refreshLabel);
    connect(nAxisButton, &JoyAxisButton::activeZoneChanged, this, &JoyAxisWidget::refreshLabel);
    connect(pAxisButton, &JoyAxisButton::activeZoneChanged, this, &JoyAxisWidget::refreshLabel);

    m_axis->establishPropertyUpdatedConnection();
    nAxisButton->establishPropertyUpdatedConnections();
    pAxisButton->establishPropertyUpdatedConnections();
}

JoyAxis *JoyAxisWidget::getAxis() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_axis;
}

void JoyAxisWidget::disableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(m_axis, &JoyAxis::active, this, &JoyAxisWidget::flash);
    disconnect(m_axis, &JoyAxis::released, this, &JoyAxisWidget::unflash);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(m_axis, &JoyAxis::active, this, &JoyAxisWidget::flash, Qt::QueuedConnection);
    connect(m_axis, &JoyAxis::released, this, &JoyAxisWidget::unflash, Qt::QueuedConnection);
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyAxisWidget::generateLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = m_axis->getName(false, ifDisplayNames()).replace("&", "&&");

    qDebug() << "Name of joy axis is: " << temp;

    return temp;
}

void JoyAxisWidget::showContextMenu(const QPoint &point)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPoint globalPos = this->mapToGlobal(point);
    JoyAxisContextMenu *contextMenu = new JoyAxisContextMenu(m_axis, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyAxisWidget::tryFlash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *nAxisButton = m_axis->getNAxisButton();
    JoyAxisButton *pAxisButton = m_axis->getPAxisButton();

    if (nAxisButton->getButtonState() || pAxisButton->getButtonState())
    {
        flash();
    }
}
