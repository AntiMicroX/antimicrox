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


#ifndef STICKPUSHBUTTONGROUP_H
#define STICKPUSHBUTTONGROUP_H

#include "joycontrolstickdirectionstype.h"

#include <QGridLayout>


class JoyControlStick;
class QWidget;
class JoyControlStickButtonPushButton;
class JoyControlStickPushButton;

class StickPushButtonGroup : public QGridLayout
{
    Q_OBJECT

public:
    explicit StickPushButtonGroup(JoyControlStick *stick, bool keypadUnlocked, bool displayNames = false, QWidget *parent = nullptr);
    JoyControlStick *getStick() const;

    bool ifDisplayNames() const;

    JoyControlStickButtonPushButton *getUpButton() const;
    JoyControlStickButtonPushButton *getDownButton() const;
    JoyControlStickButtonPushButton *getLeftButton() const;
    JoyControlStickButtonPushButton *getRightButton() const;

    JoyControlStickButtonPushButton *getUpLeftButton() const;
    JoyControlStickButtonPushButton *getUpRightButton() const;
    JoyControlStickButtonPushButton *getDownLeftButton() const;
    JoyControlStickButtonPushButton *getDownRightButton() const;

    JoyControlStickPushButton *getStickWidget() const;

protected:
    void generateButtons();

signals:
    void buttonSlotChanged();

public slots:
    void changeButtonLayout();
    void toggleNameDisplay();

private slots:
    void propogateSlotsChanged();
    void openStickButtonDialog(JoyControlStickButtonPushButton* pushbutton);
    void showStickDialog();

private:
    JoyControlStick *stick;
    bool displayNames;
    bool keypadUnlocked;

    JoyControlStickButtonPushButton *upButton;
    JoyControlStickButtonPushButton *downButton;
    JoyControlStickButtonPushButton *leftButton;
    JoyControlStickButtonPushButton *rightButton;

    JoyControlStickButtonPushButton *upLeftButton;
    JoyControlStickButtonPushButton *upRightButton;
    JoyControlStickButtonPushButton *downLeftButton;
    JoyControlStickButtonPushButton *downRightButton;

    JoyControlStickPushButton *stickWidget;

    JoyControlStickButtonPushButton * generateBtnToGrid(JoyControlStickButtonPushButton *pushbutton, JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections stickValue, int gridRow, int gridCol);
};

#endif // STICKPUSHBUTTONGROUP_H
