/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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

#include "joysensorbuttonpushbutton.h"

#include "joybuttoncontextmenu.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joysensor.h"

#include <QDebug>
#include <QMenu>
#include <QWidget>

JoySensorButtonPushButton::JoySensorButtonPushButton(JoySensorButton *button, bool displayNames, QWidget *parent)
    : FlashButtonWidget(displayNames, parent)
    , m_button(button)
{
    refreshLabel();
    enableFlashes();

    tryFlash();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &JoySensorButtonPushButton::customContextMenuRequested, this, &JoySensorButtonPushButton::showContextMenu);
    connect(m_button, &JoySensorButton::propertyUpdated, this, &JoySensorButtonPushButton::refreshLabel);
    connect(m_button, &JoySensorButton::activeZoneChanged, this, &JoySensorButtonPushButton::refreshLabel);
}

/**
 * @brief Get the JoySensorButton for this mapping
 */
JoySensorButton *JoySensorButtonPushButton::getButton() { return m_button; }

/**
 * @brief Disables highlight when the sensor axis is moved
 */
void JoySensorButtonPushButton::disableFlashes()
{
    if (m_button != nullptr)
    {
        disconnect(m_button, &JoySensorButton::clicked, this, &JoySensorButtonPushButton::flash);
        disconnect(m_button, &JoySensorButton::released, this, &JoySensorButtonPushButton::unflash);
    }
    unflash();
}

/**
 * @brief Enables highlight when the sensor axis is moved
 */
void JoySensorButtonPushButton::enableFlashes()
{
    if (m_button != nullptr)
    {
        connect(m_button, &JoySensorButton::clicked, this, &JoySensorButtonPushButton::flash, Qt::QueuedConnection);
        connect(m_button, &JoySensorButton::released, this, &JoySensorButtonPushButton::unflash, Qt::QueuedConnection);
    }
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoySensorButtonPushButton::generateLabel()
{
    QString temp = QString();
    if (m_button != nullptr)
    {
        if (!m_button->getActionName().isEmpty() && ifDisplayNames())
        {
            qDebug() << "Action name was not empty";
            temp = m_button->getActionName().replace("&", "&&");

        } else
        {
            qDebug() << "Action name was empty";
            temp = m_button->getCalculatedActiveZoneSummary().replace("&", "&&");
        }
    }

    qDebug() << "Here is name of action for pushed sensor button: " << temp;

    return temp;
}

/**
 * @brief Shows sensor context menu
 */
void JoySensorButtonPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(m_button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}

/**
 * @brief Highlights the button when mapped button is active
 */
void JoySensorButtonPushButton::tryFlash()
{
    if (m_button != nullptr && m_button->getButtonState())
        flash();
}
