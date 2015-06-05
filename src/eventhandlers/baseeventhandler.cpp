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

/**
 * @brief Do nothing by default. Useful for child classes to define behavior.
 * @param Displacement of X coordinate
 * @param Displacement of Y coordinate
 */
void BaseEventHandler::sendMouseAbsEvent(int xDis, int yDis)
{
    Q_UNUSED(xDis);
    Q_UNUSED(yDis);
}

/**
 * @brief Do nothing by default. Useful for child classes to define behavior.
 * @param Displacement of X coordinate
 * @param Displacement of Y coordinate
 * @param Bounding box width
 * @param Bounding box height
 */
void BaseEventHandler::sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                            unsigned int width, unsigned int height)
{
    Q_UNUSED(xDis);
    Q_UNUSED(yDis);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void BaseEventHandler::sendTextEntryEvent(QString maintext)
{

}
