#ifndef KEYGRABBERBUTTON_H
#define KEYGRABBERBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

//#include <X11/Xlib.h>

//The KeySym for "x"
const int XK_x = 0x078;
const int MOUSE_OFFSET = 400;

class KeyGrabberButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KeyGrabberButton(QWidget *parent = 0);
    void setValue(int value);
    int getValue ();

protected:
    void keyPressEvent(QKeyEvent *event);
    void endCountdown();
    virtual bool eventFilter(QObject *obj, QEvent *event);
    //virtual bool x11Event(XEvent *e);

    QTimer timer;
    int numSeconds;
    bool isGrabber;
    QString oldvalue;
    int oldcode;
    int controlcode;

signals:
    void grabStarted();
    void grabFinished(bool changed);
    //void grabFinished(int value);
    void mouseCode(int mousecode);
    
public slots:
    void beginCountdown();
    void updateCountdown();
};

#endif // KEYGRABBERBUTTON_H
