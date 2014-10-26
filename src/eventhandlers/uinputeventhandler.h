#ifndef UINPUTEVENTHANDLER_H
#define UINPUTEVENTHANDLER_H

#include "baseeventhandler.h"

#include <springmousemoveinfo.h>
#include <joybuttonslot.h>

class UInputEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit UInputEventHandler(QObject *parent = 0);
    ~UInputEventHandler();

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual QString getName();
    virtual QString getIdentifier();

protected:
    int openUInputHandle();
    void setKeyboardEvents(int filehandle);
    void setMouseEvents(int filehandle);
    void populateKeyCodes(int filehandle);
    void createUInputDevice(int filehandle);
    void createUInputMouseDevice(int filehandle);
    void closeUInputDevice(int filehandle);
    void write_uinput_event(int filehandle, unsigned int type,
                            unsigned int code, int value, bool syn=true);

    int keyboardFileHandler;
    int mouseFileHandler;

signals:

public slots:

private slots:
#ifdef WITH_X11
    void x11ResetMouseAccelerationChange();
#endif
};

#endif // UINPUTEVENTHANDLER_H
