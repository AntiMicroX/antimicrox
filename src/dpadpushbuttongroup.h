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


#ifndef DPADPUSHBUTTONGROUP_H
#define DPADPUSHBUTTONGROUP_H

#include <QGridLayout>

class JoyDPad;
class QWidget;
class JoyDPadButtonWidget;
class DPadPushButton;
class JoyButtonWidget;
class JoyDPadButton;

class DPadPushButtonGroup : public QGridLayout
{
    Q_OBJECT

public:
    explicit DPadPushButtonGroup(JoyDPad *dpad, bool keypadUnlocked, bool displayNames = false, QWidget *parent = nullptr);

    JoyDPad *getDPad() const;

protected:
    void generateButtons();

signals:
    void buttonSlotChanged();

public slots:
    void changeButtonLayout();
    void toggleNameDisplay();

private slots:
    void propogateSlotsChanged();
    void openDPadButtonDialog(JoyButtonWidget* buttonWidget);
    void showDPadDialog();

private:
    JoyDPad *dpad;
    bool keypadUnlocked;
    bool displayNames;

    JoyDPadButtonWidget *upButton;
    JoyDPadButtonWidget *downButton;
    JoyDPadButtonWidget *leftButton;
    JoyDPadButtonWidget *rightButton;

    JoyDPadButtonWidget *upLeftButton;
    JoyDPadButtonWidget *upRightButton;
    JoyDPadButtonWidget *downLeftButton;
    JoyDPadButtonWidget *downRightButton;

    DPadPushButton *dpadWidget;

    JoyDPadButtonWidget * generateBtnForGrid(QHash<int, JoyDPadButton*> *buttons, int dpadDirection , int cellRow, int cellCol);
};

#endif // DPADPUSHBUTTONGROUP_H
