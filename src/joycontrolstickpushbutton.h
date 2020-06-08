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


#ifndef JOYCONTROLSTICKPUSHBUTTON_H
#define JOYCONTROLSTICKPUSHBUTTON_H


#include "flashbuttonwidget.h"


class JoyControlStick;
class QWidget;

class JoyControlStickPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyControlStickPushButton(JoyControlStick *stick, bool displayNames, QWidget *parent = nullptr);

    JoyControlStick* getStick() const;
    void tryFlash();
    
protected:
    virtual QString generateLabel() override;

public slots:
    void disableFlashes() override;
    void enableFlashes() override;

private slots:
    void showContextMenu(const QPoint &point);

private:
    JoyControlStick *stick;
};

#endif // JOYCONTROLSTICKPUSHBUTTON_H
