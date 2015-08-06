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

#include "joykeyrepeathelper.h"
#include "event.h"

JoyKeyRepeatHelper::JoyKeyRepeatHelper(QObject *parent) :
    QObject(parent)
{
    lastActiveKey = 0;
    keyRepeatTimer.setParent(this);
    connect(&keyRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatKeysEvent()));
}

QTimer* JoyKeyRepeatHelper::getRepeatTimer()
{
    return &keyRepeatTimer;
}

void JoyKeyRepeatHelper::repeatKeysEvent()
{
    if (lastActiveKey)
    {
        JoyButtonSlot *slot = lastActiveKey;

        // Send another key press to fake a key repeat
        sendevent(slot);

        keyRepeatTimer.start(keyRepeatRate);
    }
    else
    {
        keyRepeatTimer.stop();
    }
}

void JoyKeyRepeatHelper::setLastActiveKey(JoyButtonSlot *slot)
{
    lastActiveKey = slot;
}

JoyButtonSlot* JoyKeyRepeatHelper::getLastActiveKey()
{
    return lastActiveKey;
}

/*void JoyKeyRepeatHelper::setKeyRepeatDelay(unsigned int repeatDelay)
{
    if (repeatDelay > 0)
    {
        keyRepeatDelay = repeatDelay;
    }
}

unsigned int JoyKeyRepeatHelper::getKeyRepeatDelay()
{
    return keyRepeatDelay;
}
*/

void JoyKeyRepeatHelper::setKeyRepeatRate(unsigned int repeatRate)
{
    if (repeatRate > 0)
    {
        keyRepeatRate = repeatRate;
    }
}

unsigned int JoyKeyRepeatHelper::getKeyRepeatRate()
{
    return keyRepeatRate;
}
