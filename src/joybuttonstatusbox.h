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

#ifndef JOYBUTTONSTATUSBOX_H
#define JOYBUTTONSTATUSBOX_H

#include <QPushButton>

#include "joybutton.h"

class JoyButtonStatusBox : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyButtonStatusBox(JoyButton *button, QWidget *parent = 0);
    JoyButton* getJoyButton();
    bool isButtonFlashing();

protected:
    JoyButton *button;
    bool isflashing;

signals:
    void flashed(bool flashing);

private slots:
    void flash();
    void unflash();
};

#endif // JOYBUTTONSTATUSBOX_H
