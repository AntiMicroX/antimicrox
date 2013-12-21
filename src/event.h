#ifndef EVENT_H
#define EVENT_H

#include <QString>

#include "joybuttonslot.h"
#include "mousehelper.h"

enum JoyInputDevice {JoyKeyboard=0, JoyMouse};

void sendevent (int code, bool pressed=true, JoyButtonSlot::JoySlotInputAction device=JoyButtonSlot::JoyKeyboard);
void sendevent(int code1, int code2);
void sendSpringEvent(double xcoor, double ycoor, int springWidth=0, int springHeight=0);
int X11KeySymToKeycode(QString key);
QString keycodeToKey(int keycode);
unsigned int X11KeyCodeToX11KeySym(unsigned int keycode);
QString keysymToKey(int keysym);

#endif // EVENT_H
