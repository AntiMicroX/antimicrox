#ifndef VIRTUALMOUSEPUSHBUTTON_H
#define VIRTUALMOUSEPUSHBUTTON_H

#include <QPushButton>
#include <QString>

#include "joybuttonslot.h"

class VirtualMousePushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit VirtualMousePushButton(QString displayText, int code, JoyButtonSlot::JoySlotInputAction mode, QWidget *parent = 0);

    unsigned int getMouseCode();
    JoyButtonSlot::JoySlotInputAction getMouseMode();


protected:
    unsigned int code;
    JoyButtonSlot::JoySlotInputAction mode;

signals:
    void mouseSlotCreated(JoyButtonSlot *tempslot);

public slots:

private slots:
    void createTempSlot();
};

#endif // VIRTUALMOUSEPUSHBUTTON_H
