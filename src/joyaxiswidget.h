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

#ifndef JOYAXISWIDGET_H
#define JOYAXISWIDGET_H

#include <QPoint>

#include "flashbuttonwidget.h"
#include "joyaxis.h"

class JoyAxisWidget : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent=0);

    JoyAxis* getAxis();
    void tryFlash();

protected:
    virtual QString generateLabel();

    JoyAxis *axis;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();

private slots:
    void showContextMenu(const QPoint &point);
};

#endif // JOYAXISWIDGET_H
