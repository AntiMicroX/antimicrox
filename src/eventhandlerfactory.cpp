#include "eventhandlerfactory.h"

EventHandlerFactory* EventHandlerFactory::instance = 0;

EventHandlerFactory::EventHandlerFactory(QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_UNIX
    #ifdef WITH_UINPUT
    eventHandler = new UInputEventHandler(this);
    #endif

    #ifdef WITH_XTEST
    eventHandler = new XTestEventHandler(this);
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

EventHandlerFactory* EventHandlerFactory::getInstance()
{
    if (!instance)
    {
        instance = new EventHandlerFactory();
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
