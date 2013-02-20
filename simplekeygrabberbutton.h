#ifndef SIMPLEKEYGRABBERBUTTON_H
#define SIMPLEKEYGRABBERBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMetaType>

#include "joybuttonslot.h"

class SimpleKeyGrabberButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SimpleKeyGrabberButton(QWidget *parent = 0);
    explicit SimpleKeyGrabberButton(int controlcode, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard, QWidget *parent = 0);

    void setValue(int value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    JoyButtonSlot* getValue();


protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    bool grabNextAction;
    bool grabbingWheel;
    JoyButtonSlot buttonslot;

signals:
    void buttonCodeChanged(int value);
    
public slots:

protected slots:

};

Q_DECLARE_METATYPE(SimpleKeyGrabberButton*)

#endif // SIMPLEKEYGRABBERBUTTON_H
