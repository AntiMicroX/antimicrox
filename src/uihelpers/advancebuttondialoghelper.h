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

#ifndef ADVANCEBUTTONDIALOGHELPER_H
#define ADVANCEBUTTONDIALOGHELPER_H

#include <QObject>

#include "joybutton.h"
#include "joybuttonslot.h"

class AdvanceButtonDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit AdvanceButtonDialogHelper(JoyButton *button,
                                       QObject *parent = 0);

protected:
    JoyButton *button;

signals:

public slots:
    void setAssignedSlot(JoyButtonSlot *otherSlot, int index);
    void setAssignedSlot(int code, unsigned int alias, int index,
                         JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void insertAssignedSlot(int code, unsigned int alias, int index,
                            JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void removeAssignedSlot(int index);
};

#endif // ADVANCEBUTTONDIALOGHELPER_H
