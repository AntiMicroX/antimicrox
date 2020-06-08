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


#ifndef JOYCONTROLSTICKBUTTONPUSHBUTTON_H
#define JOYCONTROLSTICKBUTTONPUSHBUTTON_H

#include "flashbuttonwidget.h"


class JoyControlStickButton;
class QWidget;

class JoyControlStickButtonPushButton : public FlashButtonWidget
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyControlStickButtonPushButton(JoyControlStickButton *button, bool displayNames, QWidget *parent = nullptr);

    JoyControlStickButton* getButton();
    void setButton(JoyControlStickButton *button);
    void tryFlash();

protected:
    virtual QString generateLabel() override;

public slots:
    void disableFlashes() override;
    void enableFlashes() override;

private slots:
    void showContextMenu(const QPoint &point);

private:
    JoyControlStickButton *button;
};

#endif // JOYCONTROLSTICKBUTTONPUSHBUTTON_H
