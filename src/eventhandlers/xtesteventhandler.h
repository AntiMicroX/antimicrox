#ifndef XTESTEVENTHANDLER_H
#define XTESTEVENTHANDLER_H

#include "baseeventhandler.h"

#include <joybuttonslot.h>

class XTestEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit XTestEventHandler(QObject *parent = 0);

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual QString getName();

signals:

public slots:

};

#endif // XTESTEVENTHANDLER_H
