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

#include "joytabwidgetcontainer.h"

#include "joystick.h"
#include "joytabwidget.h"

#include <QDebug>
#include <QTabBar>
#include <QWidget>

JoyTabWidgetContainer::JoyTabWidgetContainer(QWidget *parent)
    : QTabWidget(parent)
{
}

int JoyTabWidgetContainer::addTab(QWidget *widget, const QString &string) { return QTabWidget::addTab(widget, string); }

int JoyTabWidgetContainer::addTab(JoyTabWidget *widget, const QString &string)
{
    InputDevice *joystick = widget->getJoystick();

    if (joystick != nullptr)
    {
        enableFlashes(joystick);
        connect(widget, &JoyTabWidget::forceTabUnflash, this, &JoyTabWidgetContainer::unflashTab);
    }

    return QTabWidget::addTab(widget, string);
}

void JoyTabWidgetContainer::flash(InputDevice *joystick)
{
    bool found = false;

    for (int i = 0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget *>(widget(i)); // static_cast

        if ((tab != nullptr) && (tab->getJoystick() == joystick))
        {
            tabBar()->setTabTextColor(i, Qt::red);
            found = true;
        }
    }
}

void JoyTabWidgetContainer::unflash(InputDevice *joystick)
{
    bool found = false;
    for (int i = 0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget *>(widget(i)); // static_cast
        if ((tab != nullptr) && (tab->getJoystick() == joystick))
        {
            tabBar()->setTabTextColor(i, QApplication::palette().color(QPalette::Text));
            found = true;
        }
    }
}

void JoyTabWidgetContainer::unflashTab(JoyTabWidget *tabWidget)
{
    bool found = false;

    for (int i = 0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget *>(widget(i)); // static_cast
        if (tab == tabWidget)
        {
            tabBar()->setTabTextColor(i, QApplication::palette().color(QPalette::Text));
        }
    }
}

void JoyTabWidgetContainer::unflashAll()
{
    for (int i = 0; i < tabBar()->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget *>(widget(i)); // static_cast
        if (tab != nullptr)
        {
            tabBar()->setTabTextColor(i, QApplication::palette().color(QPalette::Text));
        }
    }
}

void JoyTabWidgetContainer::disableFlashes(InputDevice *joystick)
{
    unflashAll();

    disconnect(joystick, &InputDevice::clicked, this, nullptr);
    disconnect(joystick, &InputDevice::released, this, nullptr);
}

void JoyTabWidgetContainer::enableFlashes(InputDevice *joystick)
{
    connect(
        joystick, &InputDevice::clicked, this, [this, joystick] { flash(joystick); }, Qt::QueuedConnection);
    connect(
        joystick, &InputDevice::released, this, [this, joystick] { unflash(joystick); }, Qt::QueuedConnection);
}
