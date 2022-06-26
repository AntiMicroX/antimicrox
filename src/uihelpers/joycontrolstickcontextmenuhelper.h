/* antimicrox Gamepad to KB+M event mapper
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

#ifndef JOYCONTROLSTICKCONTEXTMENUHELPER_H
#define JOYCONTROLSTICKCONTEXTMENUHELPER_H

#include "joycontrolstick.h"

class JoyButtonSlot;

/**
 * @brief Some helper methods which run in the IO thread and are called
 *   from the GUI thread.
 */
class JoyControlStickContextMenuHelper : public QObject
{
    Q_OBJECT

  public:
    explicit JoyControlStickContextMenuHelper(JoyControlStick *stick, QObject *parent = nullptr);
    void setPendingSlots(QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot *> *tempSlots);
    void clearPendingSlots();
    QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot *> const &getPendingSlots();

  public slots:
    void setFromPendingSlots();
    void clearButtonsSlotsEventReset();

  private:
    JoyControlStick *stick;
    QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot *> pendingSlots;
};

#endif // JOYCONTROLSTICKCONTEXTMENUHELPER_H
