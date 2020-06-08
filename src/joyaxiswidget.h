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


#ifndef JOYAXISWIDGET_H
#define JOYAXISWIDGET_H

#include "flashbuttonwidget.h"

class JoyAxis;
class QWidget;


class JoyAxisWidget : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent=0);

    JoyAxis* getAxis() const;
    void tryFlash();

protected:
    virtual QString generateLabel() override;

public slots:
    void disableFlashes() override;
    void enableFlashes() override;

private slots:
    void showContextMenu(const QPoint &point);

private:
    JoyAxis *m_axis;
};

#endif // JOYAXISWIDGET_H
