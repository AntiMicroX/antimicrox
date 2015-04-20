#ifndef BASEEVENTHANDLER_H
#define BASEEVENTHANDLER_H

#include <QObject>
#include <QString>

#include <springmousemoveinfo.h>
#include <joybuttonslot.h>

class BaseEventHandler : public QObject
{
    Q_OBJECT
public:
    explicit BaseEventHandler(QObject *parent = 0);

    virtual bool init() = 0;
    virtual bool cleanup() = 0;
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed) = 0;
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed) = 0;
    virtual void sendMouseEvent(int xDis, int yDis) = 0;
    virtual QString getName() = 0;
    virtual QString getIdentifier() = 0;
    virtual void printPostMessages();
    QString getErrorString();
    virtual void sendMouseAbsEvent(int xDis, int yDis);
    virtual void sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                      unsigned int width, unsigned int height);

protected:
    QString lastErrorString;

signals:

public slots:

};

#endif // BASEEVENTHANDLER_H
