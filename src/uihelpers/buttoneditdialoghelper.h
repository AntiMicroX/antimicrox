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


#ifndef BUTTONEDITDIALOGHELPER_H
#define BUTTONEDITDIALOGHELPER_H

#include "joybuttonslot.h"

class JoyButton;


class ButtonEditDialogHelper : public QObject
{
    Q_OBJECT

public:
    explicit ButtonEditDialogHelper(JoyButton *button, QObject *parent = nullptr);
    explicit ButtonEditDialogHelper(QObject *parent = nullptr);
    void setThisButton(JoyButton *btn);

protected:
    JoyButton *button;

public slots:
    void setAssignedSlot(int code,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setAssignedSlot(int code, int alias,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setAssignedSlot(int code, int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setUseTurbo(bool useTurbo);
};

#endif // BUTTONEDITDIALOGHELPER_H
