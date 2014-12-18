#include "baseeventhandler.h"

BaseEventHandler::BaseEventHandler(QObject *parent) :
    QObject(parent)
{
}

QString BaseEventHandler::getErrorString()
{
    return lastErrorString;
}

/**
 * @brief Do nothing by default. Allow child classes to specify text to output
 *     to a text stream.
 */
void BaseEventHandler::printPostMessages()
{
}
