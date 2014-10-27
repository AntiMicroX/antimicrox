//#include <QDebug>

#include "joybutton.h"

#include "joybuttonmousehelper.h"

JoyButtonMouseHelper::JoyButtonMouseHelper(QObject *parent) :
    QObject(parent)
{
}

void JoyButtonMouseHelper::moveMouseCursor()
{
    //qDebug() << QTime::currentTime();

    int finalx = 0;
    int finaly = 0;
    int elapsedTime = 5;
    JoyButton::moveMouseCursor(finalx, finaly, elapsedTime);
    if (finalx != 0 || finaly != 0)
    {
        emit mouseCursorMoved(finalx, finaly, elapsedTime);
    }
}

void JoyButtonMouseHelper::moveSpringMouse()
{
    int finalx = 0;
    int finaly = 0;
    bool hasMoved = false;
    JoyButton::moveSpringMouse(finalx, finaly, hasMoved);
    if (hasMoved)
    {
        emit mouseSpringMoved(finalx, finaly);
    }
}
