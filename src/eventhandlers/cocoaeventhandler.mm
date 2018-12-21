#include "cocoaeventhandler.h"

#include <QApplication>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include <antkeymapper.h>
#include <cocoahelper.h>

CocoaEventHandler::CocoaEventHandler(QObject *parent) : BaseEventHandler(parent)
{
}

bool CocoaEventHandler::init()
{
    return true;
}

bool CocoaEventHandler::cleanup()
{
    return true;
}

void CocoaEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    int code = CocoaHelper::getInstance()->getCocoaVirtualKey(slot->getSlotCode());
    CGEventRef keyEvent = CGEventCreateKeyboardEvent(NULL, code, pressed);
    CGEventPost(kCGHIDEventTap, keyEvent);
    CFRelease(keyEvent);
}

void CocoaEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();
    if (code == 4 || code == 5){ // wheel
        CGEventRef wheel = CGEventCreateScrollWheelEvent(
                NULL,
                kCGScrollEventUnitPixel, // kCGScrollEventUnitLine,
                (CGWheelCount) 1,
                code==4?15:-15);

        CGEventPost(kCGHIDEventTap, wheel);
        CFRelease(wheel);
    }else{
        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSInteger height = screenRect.size.height;
        NSPoint mouseLoc = [NSEvent mouseLocation];
        CGEventType eventType;
        CGMouseButton buttonType;
        if (code == 1)
        {
            eventType = pressed ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
            buttonType = kCGMouseButtonLeft;
        }
        else if (code == 2)
        {
            eventType = pressed ? kCGEventOtherMouseDown : kCGEventOtherMouseUp;
            buttonType = kCGMouseButtonCenter;
        }
        else if (code == 3)
        {
            eventType = pressed ? kCGEventRightMouseDown : kCGEventRightMouseUp;
            buttonType = kCGMouseButtonRight;
        }

        /* CGEventType eventType = pressed ? kCGEventLeftMouseDown : kCGEventLeftMouseUp; */
        CGEventRef click = CGEventCreateMouseEvent(
                NULL,
                eventType,
                CGPointMake(mouseLoc.x, height - mouseLoc.y),
                buttonType);
        CGEventPost(kCGHIDEventTap, click);
        CFRelease(click);
    }
}

void CocoaEventHandler::sendMouseEvent(int xDis, int yDis)
{
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSInteger height = screenRect.size.height;
    NSPoint mouseLoc = [NSEvent mouseLocation];

    mouseLoc.x += xDis;
    mouseLoc.y -= yDis;

    CGEventRef move = CGEventCreateMouseEvent(
            NULL,
            kCGEventMouseMoved,
            CGPointMake(mouseLoc.x, height - mouseLoc.y),
            kCGMouseButtonLeft);
    CGEventSetType(move, kCGEventMouseMoved);
    CGEventSetIntegerValueField(move, kCGMouseEventDeltaX, xDis);
    CGEventSetIntegerValueField(move, kCGMouseEventDeltaY, yDis);

    CGEventPost(kCGHIDEventTap, move);
    CFRelease(move);
}

void CocoaEventHandler::sendMouseAbsEvent(int xDis, int yDis, int screen)
{
    // Not implemented
}

QString CocoaEventHandler::getName()
{
    return QString("Cocoa");
}

QString CocoaEventHandler::getIdentifier()
{
    return QString("cocoa");
}

void CocoaEventHandler::sendTextEntryEvent(QString maintext)
{
    // Not implemented
}
