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

#ifndef STICKPUSHBUTTONGROUP_H
#define STICKPUSHBUTTONGROUP_H

#include <QGridLayout>

#include "joycontrolstick.h"
#include "joycontrolstickpushbutton.h"
#include "joycontrolstickbuttonpushbutton.h"

class StickPushButtonGroup : public QGridLayout
{
    Q_OBJECT
public:
    explicit StickPushButtonGroup(JoyControlStick *stick, bool displayNames = false, QWidget *parent = 0);
    JoyControlStick *getStick();

protected:
    void generateButtons();

    JoyControlStick *stick;
    bool displayNames;

    JoyControlStickButtonPushButton *upButton;
    JoyControlStickButtonPushButton *downButton;
    JoyControlStickButtonPushButton *leftButton;
    JoyControlStickButtonPushButton *rightButton;

    JoyControlStickButtonPushButton *upLeftButton;
    JoyControlStickButtonPushButton *upRightButton;
    JoyControlStickButtonPushButton *downLeftButton;
    JoyControlStickButtonPushButton *downRightButton;

    JoyControlStickPushButton *stickWidget;

signals:
    void buttonSlotChanged();

public slots:
    void changeButtonLayout();
    void toggleNameDisplay();

private slots:
    void propogateSlotsChanged();
    void openStickButtonDialog();
    void showStickDialog();
};

#endif // STICKPUSHBUTTONGROUP_H
