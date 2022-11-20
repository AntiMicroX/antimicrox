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

#include "joydpadbuttonwidget.h"

#include "joybuttontypes/joybutton.h"

#include <QDebug>
#include <QWidget>

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, bool displayNames, QWidget *parent)
    : JoyButtonWidget(button, displayNames, parent)
{ // Ensure that JoyDPadButtonWidget::generateLabel is called.
    refreshLabel();
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyDPadButtonWidget::generateLabel()
{
    QString temp = QString();
    if (!getJoyButton()->getActionName().isEmpty() && ifDisplayNames())
    {
        temp = getJoyButton()->getActionName();
    } else
    {
        temp = getJoyButton()->getCalculatedActiveZoneSummary();
    }
    temp.replace("&", "&&");

    qDebug() << "Name of joy dpad button is: " << temp;

    return temp;
}
