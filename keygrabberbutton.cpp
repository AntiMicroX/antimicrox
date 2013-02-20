#include <QDebug>

#include "keygrabberbutton.h"
#include "event.h"

KeyGrabberButton::KeyGrabberButton(QWidget *parent) :
    QPushButton(parent)
{
    numSeconds = 5;
    isGrabber = false;
    controlcode = 0;
    grabbingWheel = false;
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

    QPushButton::keyPressEvent(event);
}

void KeyGrabberButton::beginCountdown()
{
    disconnect(this, SIGNAL(clicked()), 0, 0);

    this->grabKeyboard();

    emit grabStarted();

    isGrabber = true;
    oldLabel = this->text();
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
        this->setText(oldLabel);
        emit grabFinished(false);
    }
}

void KeyGrabberButton::endCountdown()
{
    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), 0, 0);

    this->releaseKeyboard();
    numSeconds = 5;
    isGrabber = grabbingWheel = false;

    connect (this, SIGNAL(clicked()), this, SLOT(beginCountdown()));
}

bool KeyGrabberButton::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

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

        controlcode = mouseValue;
        buttonslot.setSlotCode(mouseValue);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMouseButton);
        clearFocus();
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

        buttonslot.setSlotCode(controlcode);
        buttonslot.setSlotMode(JoyButtonSlot::JoyKeyboard);
        clearFocus();
        this->endCountdown();
        emit grabFinished(true);
    }
    else if (isGrabber && event->type() == QEvent::Wheel && !grabbingWheel)
    {
        grabbingWheel = true;
    }
    else if (isGrabber && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEve = (QWheelEvent*) event;
        QString text = QString("Mouse ");

        if (wheelEve->orientation() == Qt::Vertical && wheelEve->delta() >= 120)
        {
            controlcode = 4;
        }
        else if (wheelEve->orientation() == Qt::Vertical && wheelEve->delta() <= -120)
        {
            controlcode = 5;
        }

        text = text.append(QString::number(controlcode));
        setText(text);

        buttonslot.setSlotCode(controlcode);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMouseButton);
        clearFocus();
        this->endCountdown();
        emit grabFinished(true);
    }

    return false;
}

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

void KeyGrabberButton::setValue(int value, JoyButtonSlot::JoySlotInputAction mode)
{
    controlcode = value;
    buttonslot.setSlotCode(controlcode);
    buttonslot.setSlotMode(mode);
}

JoyButtonSlot* KeyGrabberButton::getValue()
{
    return &buttonslot;
}
