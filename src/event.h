#ifndef EVENT_H
#define EVENT_H

#include <QString>

#include "joybuttonslot.h"
#include "mousehelper.h"
#include "springmousemoveinfo.h"

enum JoyInputDevice {JoyKeyboard=0, JoyMouse};

void sendevent (JoyButtonSlot *slot, bool pressed=true);
void sendevent(int code1, int code2);
void sendSpringEvent(PadderCommon::springModeInfo *fullSpring, PadderCommon::springModeInfo *relativeSpring=0);
//void sendSpringEvent(double xcoor, double ycoor, int springWidth=0, int springHeight=0);
int X11KeySymToKeycode(QString key);
QString keycodeToKeyString(int keycode, unsigned int alias=0);
unsigned int X11KeyCodeToX11KeySym(unsigned int keycode);
QString keysymToKeyString(int keysym, unsigned int alias=0);

#endif // EVENT_H
