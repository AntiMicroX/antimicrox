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

#include "joytabwidgetcontainer.h"
#include "joystick.h"
#include "joytabwidget.h"

#include <QWidget>
#include <QTabBar>
#include <QDebug>

JoyTabWidgetContainer::JoyTabWidgetContainer(QWidget *parent) :
    QTabWidget(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

}

int JoyTabWidgetContainer::addTab(QWidget *widget, const QString &string)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return QTabWidget::addTab(widget, string);
}

int JoyTabWidgetContainer::addTab(JoyTabWidget *widget, const QString &string)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    InputDevice *joystick = widget->getJoystick();

    if (joystick != nullptr)
    {
        enableFlashes(joystick);
        connect(widget, SIGNAL(forceTabUnflash(JoyTabWidget*)), this, SLOT(unflashTab(JoyTabWidget*)));
    }

    return QTabWidget::addTab(widget, string);
}

void JoyTabWidgetContainer::flash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    InputDevice *joystick = qobject_cast<InputDevice*>(sender()); // static_cast

    bool found = false;
    for (int i = 0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(widget(i)); // static_cast
        if ((tab != nullptr) && (tab->getJoystick() == joystick))
        {
            tabBar()->setTabTextColor(i, Qt::red);
            found = true;
        }
    }
}

void JoyTabWidgetContainer::unflash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    InputDevice *joystick = qobject_cast<InputDevice*>(sender()); // static_cast

    bool found = false;
    for (int i = 0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(widget(i)); // static_cast
        if ((tab != nullptr) && (tab->getJoystick() == joystick))
        {
            tabBar()->setTabTextColor(i, Qt::black);
            found = true;
        }
    }
}

void JoyTabWidgetContainer::unflashTab(JoyTabWidget *tabWidget)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool found = false;

    for (int i=0; (i < tabBar()->count()) && !found; i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(widget(i)); // static_cast
        if (tab == tabWidget)
        {
            tabBar()->setTabTextColor(i, Qt::black);
        }
    }
}

void JoyTabWidgetContainer::unflashAll()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    for (int i = 0; i < tabBar()->count(); i++)
    {
        JoyTabWidget *tab = qobject_cast<JoyTabWidget*>(widget(i)); // static_cast
        if (tab != nullptr)
        {
            tabBar()->setTabTextColor(i, Qt::black);
        }
    }
}

void JoyTabWidgetContainer::disableFlashes(InputDevice *joystick)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    unflashAll();

    disconnect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(joystick, SIGNAL(released(int)), this, SLOT(unflash()));
}

void JoyTabWidgetContainer::enableFlashes(InputDevice *joystick)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    connect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(joystick, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}
