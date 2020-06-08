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


#ifndef DPADPUSHBUTTON_H
#define DPADPUSHBUTTON_H


#include "flashbuttonwidget.h"


class JoyDPad;
class QWidget;

class DPadPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent = nullptr);

    JoyDPad* getDPad() const;
    void tryFlash();

protected:
    QString generateLabel() override;

public slots:
    void disableFlashes() override;
    void enableFlashes() override;

private slots:
    void showContextMenu(const QPoint &point);

private:
    JoyDPad *dpad;

};

#endif // DPADPUSHBUTTON_H
