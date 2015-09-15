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

#include "joyaxiscontextmenu.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->axis = axis;

    refreshLabel();
    enableFlashes();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    tryFlash();

    connect(axis, SIGNAL(throttleChanged()), this, SLOT(refreshLabel()));
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(refreshLabel()));

    //connect(nAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    //connect(pAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));

    axis->establishPropertyUpdatedConnection();
    nAxisButton->establishPropertyUpdatedConnections();
    pAxisButton->establishPropertyUpdatedConnections();
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyAxisWidget::generateLabel()
{
    QString temp;
    temp = axis->getName(false, displayNames).replace("&", "&&");
    return temp;
}

void JoyAxisWidget::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    JoyAxisContextMenu *contextMenu = new JoyAxisContextMenu(axis, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

void JoyAxisWidget::tryFlash()
{
    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    if (nAxisButton->getButtonState() || pAxisButton->getButtonState())
    {
        flash();
    }
}
