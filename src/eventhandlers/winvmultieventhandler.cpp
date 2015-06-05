//#include <QDebug>
#include <qt_windows.h>
#include <winextras.h>

#include "winvmultieventhandler.h"

#include <vmulticlient.h>
#include <qtvmultikeymapper.h>
#include <antkeymapper.h>

WinVMultiEventHandler::WinVMultiEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
    vmulti = 0;
    mouseButtons = 0;
    shiftKeys = 0;
    multiKeys = 0;
    extraKeys = 0;

    keyboardKeys.resize(6);
    keyboardKeys.fill(0);
}

WinVMultiEventHandler::~WinVMultiEventHandler()
{
    cleanup();
}

bool WinVMultiEventHandler::init()
{
    bool result = true;

    vmulti = vmulti_alloc();

    if (vmulti == NULL)
    {
        result = false;
    }

    if (vmulti && !vmulti_connect(vmulti))
    {
        vmulti_free(vmulti);
        vmulti = 0;
        result = false;
    }

    return result;
}

bool WinVMultiEventHandler::cleanup()
{
    bool result = true;

    if (vmulti)
    {
        vmulti_disconnect(vmulti);
        vmulti_free(vmulti);

        vmulti = 0;
    }

    return result;
}

void WinVMultiEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    int code = slot->getSlotCode();

    BYTE pendingShift = 0x0;
    BYTE pendingMultimedia = 0x0;
    BYTE pendingExtra = 0x0;
    BYTE pendingKey = 0x0;

    bool exists = keyboardKeys.contains(code);

    if (code <= 0x65)
    {
        pendingKey = code;
    }
    else if (code >= 0xE0 && code <= 0xE7)
    {
        pendingShift = 1 << (code - 0xE0);
    }
    else if (code > QtVMultiKeyMapper::consumerUsagePagePrefix)
    {
        if (code == 0xB5 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingMultimedia = 1 << 0; // (Scan Next Track)
        }
        else if (code == 0xB6 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingMultimedia = 1 << 1; // (Scan Previous Track)
        }
        else if (code == 0xB1 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingMultimedia = 1 << 3; // (Play / Pause)
        }
        else if (code == 0x189 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingMultimedia = 1 << 6; // (WWW Home)
        }

        else if (code == 0x194 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 0; // (My Computer)
        }
        else if (code == 0x192 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 1; // (Calculator)
        }
        else if (code == 0x22a | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 2; // (WWW fav)
        }
        else if (code == 0x221 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 3; // (WWW search)
        }
        else if (code == 0xB7 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 3; // (WWW stop)
        }
        else if (code == 0x224 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 4; // (WWW back)
        }
        else if (code == 0x87 | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 5; // (Media Select)
        }
        else if (code == 0x18a | QtVMultiKeyMapper::consumerUsagePagePrefix)
        {
            pendingExtra = 1 << 6; // (Mail)
        }

    }
    else if (code > 0x65)
    {
        if (code == 0x78)
        {
            pendingMultimedia = 1 << 2; // (Stop)
        }
        else if (code == 0x7F)
        {
            pendingMultimedia = 1 << 4; // (Mute)
        }
        else if (code == 0x81)
        {
            pendingMultimedia = 1 << 5; // (Volume Down)
        }
        else if (code == 0x80)
        {
            pendingMultimedia = 1 << 6; // (Volume Up)
        }
    }

    if (pressed)
    {
        shiftKeys = shiftKeys | pendingShift;
        multiKeys = multiKeys | pendingMultimedia;
        extraKeys = extraKeys | pendingExtra;

        if (!exists)
        {
            // Check for an empty key value
            int index = keyboardKeys.indexOf(0);
            if (index != -1)
            {
                keyboardKeys.replace(index, pendingKey);
            }
        }
    }
    else
    {
        shiftKeys = shiftKeys ^ pendingShift;
        multiKeys = multiKeys ^ pendingMultimedia;
        extraKeys = extraKeys ^ pendingExtra;

        if (exists)
        {
            int index = keyboardKeys.indexOf(pendingKey);
            if (index != -1)
            {
                keyboardKeys.replace(index, 0);
            }
        }
    }

    BYTE *keykeyArray = keyboardKeys.data();
    /*QStringList trying;
    for (int i=0; i < 6; i++)
    {
        BYTE current = keykeyArray[i];
        trying.append(QString("0x%1").arg(QString::number(current, 16)));
    }
    */

    //qDebug() << "CURRENT: " << trying.join(",");
    //qDebug() << keykeyArray;

    if (pendingKey > 0x0)
    {
        vmulti_update_keyboard(vmulti, shiftKeys, keykeyArray);
    }

    if (pendingMultimedia > 0 || pendingExtra > 0)
    {
        vmulti_update_keyboard_enhanced(vmulti, multiKeys, extraKeys);
    }
}

void WinVMultiEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    BYTE pendingButton = 0;
    BYTE pendingWheel = 0;
    BYTE pendingHWheel = 0;

    int code = slot->getSlotCode();
    if (code == 1)
    {
        pendingButton = 0x01;
    }
    else if (code == 2)
    {
        pendingButton = 0x04;
    }
    else if (code == 3)
    {
        pendingButton = 0x02;
    }
    else if (code == 4)
    {
        pendingWheel = pressed ? 1 : 0;
    }
    else if (code == 5)
    {
        pendingWheel = pressed ? -1 : 0;
    }
    else if (code == 6)
    {
        pendingHWheel = pressed ? -1 : 0;
    }
    else if (code == 7)
    {
        pendingHWheel = pressed ? 1 : 0;
    }
    else if (code == 8)
    {
        pendingButton = 0x08;
    }
    else if (code == 9)
    {
        pendingButton = 0x10;
    }

    if (pressed)
    {
        mouseButtons = mouseButtons | pendingButton;
        vmulti_update_relative_mouse(vmulti, mouseButtons, 0, 0, pendingWheel, pendingHWheel);
    }
    else
    {
        mouseButtons = mouseButtons ^ pendingButton;
        vmulti_update_relative_mouse(vmulti, mouseButtons, 0, 0, pendingWheel, pendingHWheel);
    }
}

void WinVMultiEventHandler::sendMouseEvent(int xDis, int yDis)
{
    vmulti_update_relative_mouse(vmulti, mouseButtons, xDis, yDis, 0, 0);
}

void WinVMultiEventHandler::sendMouseAbsEvent(int xDis, int yDis)
{
    vmulti_update_mouse(vmulti, mouseButtons, xDis, yDis, 0);
}

/*
 * TODO: Implement text event using information from QtWinKeyMapper.
 */
void WinVMultiEventHandler::sendTextEntryEvent(QString maintext)
{

}

QString WinVMultiEventHandler::getName()
{
    return QString("Vmulti");
}

QString WinVMultiEventHandler::getIdentifier()
{
    return QString("vmulti");
}
