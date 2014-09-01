#include "baseeventhandler.h"

BaseEventHandler::BaseEventHandler(QObject *parent) :
    QObject(parent)
{
}

QString BaseEventHandler::getErrorString()
{
    return lastErrorString;
}
