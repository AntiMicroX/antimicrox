#ifndef KEYGRABBERBUTTON_H
#define KEYGRABBERBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

#include "joybuttonslot.h"

const int MOUSE_OFFSET = 400;

class KeyGrabberButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KeyGrabberButton(QWidget *parent = 0);

    void setValue(int value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    JoyButtonSlot* getValue();

protected:
    void keyPressEvent(QKeyEvent *event);
    void endCountdown();
    virtual bool eventFilter(QObject *obj, QEvent *event);

    QTimer timer;
    int numSeconds;
    bool isGrabber;
    QString oldLabel;
    int controlcode;
    JoyButtonSlot buttonslot;
    bool grabbingWheel;

signals:
    void grabStarted();
    void grabFinished(bool changed);
    void mouseCode(int mousecode);
    
public slots:
    void beginCountdown();
    void updateCountdown();
};

#endif // KEYGRABBERBUTTON_H
