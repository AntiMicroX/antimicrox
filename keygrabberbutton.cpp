#include <QDebug>

#include "keygrabberbutton.h"
#include "event.h"

KeyGrabberButton::KeyGrabberButton(QWidget *parent) :
    QPushButton(parent)
{
    numSeconds = 5;
    isGrabber = false;
    oldcode = 0;
    controlcode = 0;
    connect (this, SIGNAL(clicked()), this, SLOT(beginCountdown()));
    this->installEventFilter(this);
}

void KeyGrabberButton::keyPressEvent(QKeyEvent *event)
{
    // Do not allow closing of dialog using Escape key
    if (event->key() == Qt::Key_Escape)
    {
        return;
    }
}

void KeyGrabberButton::beginCountdown()
{
    disconnect(this, SIGNAL(clicked()), 0, 0);

    this->grabKeyboard();

    emit grabStarted();

    isGrabber = true;
    oldvalue = this->text();
    this->setText(QString ("[%1]").arg(numSeconds));
    connect (&timer, SIGNAL(timeout()), this, SLOT(updateCountdown()));
    timer.start(1000);
}

void KeyGrabberButton::updateCountdown()
{
    if (numSeconds > 0)
    {
        numSeconds--;
        this->setText(QString ("[%1]").arg(numSeconds));
    }
    else
    {
        endCountdown();
        this->setText(oldvalue);
        emit grabFinished(false);
    }
}

void KeyGrabberButton::endCountdown()
{
    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), 0, 0);

    this->releaseKeyboard();
    numSeconds = 5;
    isGrabber = false;

    connect (this, SIGNAL(clicked()), this, SLOT(beginCountdown()));
}

bool KeyGrabberButton::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_UNUSED(event);

    if (isGrabber && event->type() == QEvent::MouseButtonRelease)
    {
        int mouseValue = 0;
        QMouseEvent *mouseEve = (QMouseEvent*) event;

        if (mouseEve->button() == Qt::RightButton)
        {
            mouseValue = 3;
        }
        else if (mouseEve->button() == Qt::MiddleButton)
        {
            mouseValue = 2;
        }
        else {
            mouseValue = mouseEve->button();
        }

        setText(QString("Mouse ").append(QString::number(mouseValue)));
        controlcode = mouseValue + MOUSE_OFFSET;

        this->clearFocus();
        this->endCountdown();
        emit grabFinished(true);
    }
    else if (isGrabber && event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEve = (QKeyEvent*) event;
        //qDebug() << "EVENT: " << keyEve->nativeVirtualKey() << endl;
        //qDebug() << "EVENT 2: " << keyEve->nativeScanCode() << endl;
        controlcode = keyEve->nativeScanCode();

        if ((keyEve->modifiers() & Qt::ControlModifier) && keyEve->key() == Qt::Key_X)
        {
            controlcode = 0;
            setText("[NO KEY]");
        }
        else if (controlcode < 0)
        {
            controlcode = 0;
            setText("[NO KEY]");
        }
        else
        {
            setText(keycodeToKey(controlcode).toUpper());
        }

        this->clearFocus();
        this->endCountdown();
        emit grabFinished(true);
    }
    else if (isGrabber && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEve = (QWheelEvent*) event;
        QString text = QString("Mouse ");
        if (wheelEve->delta() > 0)
        {
            controlcode = 4;
        }
        else
        {
            controlcode = 5;
        }

        text = text.append(QString::number(controlcode));
        setText(text);

        controlcode += MOUSE_OFFSET;
        this->clearFocus();
        this->endCountdown();
        emit grabFinished(true);
    }

    return false;
}

/*bool KeyGrabberButton::x11Event(XEvent *e)
{
    //QPushButton::x11Event(e);
    qDebug () << "jfdsjfsdjkl: " << e->xbutton.button << endl;
}*/

/*bool KeyGrabberButton::x11Event(XEvent *e)
{
    //On a key press, return the key and quit
    //Ctrl+X == [No Key]
    if (e->type == KeyRelease) {
        if ((int)XKeycodeToKeysym(e->xkey.display,e->xkey.keycode,0) == XK_x ) {
            if (e->xkey.state & ControlMask)
            {
                controlcode = 0;
            }
            else
            {
                controlcode = e->xkey.keycode;
            }
        }

        else
        {
            controlcode = e->xkey.keycode;
        }

        if (controlcode == 0)
        {
            setText("[NO KEY]");
        }
        else
        {
            setText(keycodeToKey(controlcode).toUpper());
        }

        this->clearFocus();
        this->endCountdown();
        emit grabFinished(true);

        return true;
    }
    //if we're accepting mouse clicks and a mouse button was clicked...
    else if (e->type == ButtonRelease) {
        setText(QString("Mouse ").append(QString::number(controlcode)));

        controlcode = e->xbutton.button + MOUSE_OFFSET;

        this->clearFocus();
        this->endCountdown();
        emit grabFinished(true);

        return true;
    }

    //any other events we will pass on to the dialog. This allows for closing
    //the window and easy redrawing  :)
    return false;
}*/

int KeyGrabberButton::getValue()
{
    return controlcode;
}

void KeyGrabberButton::setValue(int value)
{
    controlcode = value;
}
