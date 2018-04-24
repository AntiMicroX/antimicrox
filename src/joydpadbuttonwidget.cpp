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

#include "joydpadbuttonwidget.h"
#include "joybutton.h"

#include <QWidget>
#include <QDebug>

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    JoyButtonWidget(button, displayNames, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    // Ensure that JoyDPadButtonWidget::generateLabel is called.
    refreshLabel();
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyDPadButtonWidget::generateLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (!getJoyButton()->getActionName().isEmpty() && displayNames)
    {
        temp = getJoyButton()->getActionName();
    }
    else
    {
        temp = getJoyButton()->getCalculatedActiveZoneSummary();
    }
    temp.replace("&", "&&");

    qDebug() << "Name of joy dpad button is: " << temp;
    return temp;
}
