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

#include "joybuttonstatusbox.h"
#include "joybutton.h"

#include <QWidget>
#include <QStyle>
#include <QDebug>

JoyButtonStatusBox::JoyButtonStatusBox(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->button = button;
    isflashing = false;

    setText(QString::number(button->getRealJoyNumber()));

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyButton* JoyButtonStatusBox::getJoyButton() const
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return button;
}

bool JoyButtonStatusBox::isButtonFlashing()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return isflashing;
}

void JoyButtonStatusBox::flash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonStatusBox::unflash()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}
