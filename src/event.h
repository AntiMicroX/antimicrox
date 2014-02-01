#ifndef EVENT_H
#define EVENT_H

#include <QString>

#include "joybuttonslot.h"
#include "mousehelper.h"

enum JoyInputDevice {JoyKeyboard=0, JoyMouse};

void sendevent (JoyButtonSlot *slot, bool pressed=true);
void sendevent(int code1, int code2);
void sendSpringEvent(double xcoor, double ycoor, int springWidth=0, int springHeight=0);
int X11KeySymToKeycode(QString key);
QString keycodeToKey(int keycode, unsigned int alias=0);
unsigned int X11KeyCodeToX11KeySym(unsigned int keycode);
QString keysymToKey(int keysym, unsigned int alias=0);

#endif // EVENT_H
