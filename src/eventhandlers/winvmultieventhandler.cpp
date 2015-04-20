//#include <QDebug>
#include <qt_windows.h>

#include "winvmultieventhandler.h"

#include <winextras.h>
#include <vmulticlient.h>

WinVMultiEventHandler::WinVMultiEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
    vmulti = 0;
    mouseButtons = 0;
    shiftKeys = 0;
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

    bool exists = keyboardKeys.contains(code);
    if (pressed)
    {
        if (code >= 0xE0 && code <= 0xE7)
        {
            shiftKeys = shiftKeys | (1 << (code - 0xE0));
        }
        else if (!exists)
        {
            int index = keyboardKeys.indexOf(0);
            if (index != -1)
            {
                keyboardKeys.replace(index, code);
            }
        }
    }
    else
    {
        if (code >= 0xE0 && code <= 0xE7)
        {
            shiftKeys = shiftKeys ^ (1 << (code - 0xE0));
        }
        else if (exists)
        {
            int index = keyboardKeys.indexOf(code);
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

    vmulti_update_keyboard(vmulti, shiftKeys, keykeyArray);
}

void WinVMultiEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    BYTE pendingButton = 0;
    BYTE pendingWheel = 0;

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
    /*else if (code == 6)
    {

    }
    else if (code == 7)
    {

    }
    */
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
        vmulti_update_relative_mouse(vmulti, mouseButtons, 0, 0, pendingWheel);
    }
    else
    {
        mouseButtons = mouseButtons ^ pendingButton;
        vmulti_update_relative_mouse(vmulti, mouseButtons, 0, 0, pendingWheel);
    }
}

void WinVMultiEventHandler::sendMouseEvent(int xDis, int yDis)
{
    vmulti_update_relative_mouse(vmulti, mouseButtons, xDis, yDis, 0);
}

void WinVMultiEventHandler::sendMouseAbsEvent(int xDis, int yDis)
{
    vmulti_update_mouse(vmulti, mouseButtons, xDis, yDis, 0);
}

QString WinVMultiEventHandler::getName()
{
    return QString("Vmulti");
}

QString WinVMultiEventHandler::getIdentifier()
{
    return QString("vmulti");
}
