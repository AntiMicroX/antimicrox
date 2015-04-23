#include "joykeyrepeathelper.h"
#include "event.h"

JoyKeyRepeatHelper::JoyKeyRepeatHelper(QObject *parent) :
    QObject(parent)
{
    lastActiveKey = 0;
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
