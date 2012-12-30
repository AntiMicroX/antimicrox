#ifndef EVENT_H
#define EVENT_H

#include <QString>

enum JoyInputDevice {JoyKeyboard=0, JoyMouse};

void sendevent (int code, bool pressed=true, JoyInputDevice device=JoyKeyboard);
void sendevent(int code1, int code2);
int keyToKeycode (QString key);
QString keycodeToKey(int keycode);

#endif // EVENT_H
