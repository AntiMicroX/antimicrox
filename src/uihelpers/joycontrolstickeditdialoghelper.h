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

#ifndef JOYCONTROLSTICKEDITDIALOGHELPER_H
#define JOYCONTROLSTICKEDITDIALOGHELPER_H

#include <QObject>
#include <QHash>

#include "joycontrolstick.h"

class JoyControlStickEditDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit JoyControlStickEditDialogHelper(JoyControlStick *stick, QObject *parent = 0);
    void setPendingSlots(QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot*> *tempSlots);
    void clearPendingSlots();

protected:
    JoyControlStick *stick;
    QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot*> pendingSlots;

signals:

public slots:
    void setFromPendingSlots();
    void clearButtonsSlotsEventReset();
    void updateControlStickDelay(int value);
};

#endif // JOYCONTROLSTICKEDITDIALOGHELPER_H
