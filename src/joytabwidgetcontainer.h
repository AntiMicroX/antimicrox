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

#ifndef JOYTABWIDGETCONTAINER_H
#define JOYTABWIDGETCONTAINER_H

#include <QTabWidget>

#include "joystick.h"
#include "joytabwidget.h"

class JoyTabWidgetContainer : public QTabWidget
{
    Q_OBJECT
public:
    explicit JoyTabWidgetContainer(QWidget *parent = 0);

    int addTab(QWidget *widget, const QString &string);
    int addTab(JoyTabWidget *widget, const QString &string);

protected:

signals:

public slots:
    void disableFlashes(InputDevice *joystick);
    void enableFlashes(InputDevice *joystick);

private slots:
    void flash();
    void unflash();
    void unflashAll();
    void unflashTab(JoyTabWidget *tabWidget);
};

#endif // JOYTABWIDGETCONTAINER_H
