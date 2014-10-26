#include <QStringList>

#include "eventhandlerfactory.h"

static QStringList buildEventGeneratorList()
{
    QStringList temp;

    temp.append("xtest");
    temp.append("uinput");
    return temp;
}

EventHandlerFactory* EventHandlerFactory::instance = 0;

EventHandlerFactory::EventHandlerFactory(QString handler, QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_UNIX
    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        eventHandler = new UInputEventHandler(this);
    }
    #endif

    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        eventHandler = new XTestEventHandler(this);
    }
    #endif

#endif
}

EventHandlerFactory::~EventHandlerFactory()
{
    if (eventHandler)
    {
        delete eventHandler;
        eventHandler = 0;
    }
}

EventHandlerFactory* EventHandlerFactory::getInstance(QString handler)
{
    if (!instance)
    {
        QStringList temp = buildEventGeneratorList();
        if (temp.contains(handler))
        {
            instance = new EventHandlerFactory(handler);
        }
    }

    return instance;
}

void EventHandlerFactory::deleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

BaseEventHandler* EventHandlerFactory::handler()
{
    return eventHandler;
}
