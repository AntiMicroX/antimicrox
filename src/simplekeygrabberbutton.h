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

    void setValue(int value, unsigned int alias, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setValue(int value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    JoyButtonSlot* getValue();
    bool isEdited();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    bool grabNextAction;
    bool grabbingWheel;
    bool edited;
    JoyButtonSlot buttonslot;

signals:
    void buttonCodeChanged(int value);
    
public slots:
    void refreshButtonLabel();

protected slots:

};

Q_DECLARE_METATYPE(SimpleKeyGrabberButton*)

#endif // SIMPLEKEYGRABBERBUTTON_H
