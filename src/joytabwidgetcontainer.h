/* antimicroX Gamepad to KB+M event mapper
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


#ifndef JOYTABWIDGETCONTAINER_H
#define JOYTABWIDGETCONTAINER_H

#include <QTabWidget>


class QWidget;
class JoyTabWidget;
class InputDevice;

class JoyTabWidgetContainer : public QTabWidget
{
    Q_OBJECT

public:
    explicit JoyTabWidgetContainer(QWidget *parent = nullptr);

    int addTab(QWidget *widget, const QString &string);
    int addTab(JoyTabWidget *widget, const QString &string);

public slots:
    void disableFlashes(InputDevice *joystick);
    void enableFlashes(InputDevice *joystick);

private slots:
    void flash(InputDevice* joystick);
    void unflash(InputDevice *joystick);
    void unflashAll();
    void unflashTab(JoyTabWidget *tabWidget);
};

#endif // JOYTABWIDGETCONTAINER_H
