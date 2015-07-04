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

#include <QStyle>

#include "joybuttonstatusbox.h"

JoyButtonStatusBox::JoyButtonStatusBox(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{
    this->button = button;
    isflashing = false;

    setText(QString::number(button->getRealJoyNumber()));

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyButton* JoyButtonStatusBox::getJoyButton()
{
    return button;
}

bool JoyButtonStatusBox::isButtonFlashing()
{
    return isflashing;
}

void JoyButtonStatusBox::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonStatusBox::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}
