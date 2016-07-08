#ifndef COCOAEVENTHANDLER_H
#define COCOAEVENTHANDLER_H

#include "baseeventhandler.h"

#include <joybuttonslot.h>

class CocoaEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit CocoaEventHandler(QObject *parent = 0);

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual void sendMouseAbsEvent(int xDis, int yDis, int screen);

    virtual QString getName();
    virtual QString getIdentifier();

    virtual void sendTextEntryEvent(QString maintext);

signals:

public slots:

};

#endif // COCOAEVENTHANDLER_H
