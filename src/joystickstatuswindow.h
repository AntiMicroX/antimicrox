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


#ifndef JOYSTICKSTATUSWINDOW_H
#define JOYSTICKSTATUSWINDOW_H

#include <QDialog>

class InputDevice;
class QWidget;

namespace Ui {
class JoystickStatusWindow;
}

class JoystickStatusWindow : public QDialog
{
    Q_OBJECT

public:
    explicit JoystickStatusWindow(InputDevice *joystick, QWidget *parent = nullptr);
    ~JoystickStatusWindow();

    InputDevice* getJoystick() const;

private:
    Ui::JoystickStatusWindow *ui;

    InputDevice *joystick;

private slots:
    void restoreButtonStates(int code);
    void obliterate();
};

#endif // JOYSTICKSTATUSWINDOW_H
