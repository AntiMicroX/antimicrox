#ifndef WINVMULTIEVENTHANDLER_H
#define WINVMULTIEVENTHANDLER_H

#include <QObject>
#include <QVector>

#include "baseeventhandler.h"

#include <joybuttonslot.h>

#include <vmulticlient.h>

class WinVMultiEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit WinVMultiEventHandler(QObject *parent = 0);
    ~WinVMultiEventHandler();

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual void sendMouseAbsEvent(int xDis, int yDis);
    virtual QString getName();
    virtual QString getIdentifier();

protected:
    pvmulti_client vmulti;
    BYTE mouseButtons;
    BYTE shiftKeys;
    BYTE multiKeys;
    BYTE extraKeys;
    QVector<BYTE> keyboardKeys;

signals:

public slots:

};

#endif // WINVMULTIEVENTHANDLER_H
