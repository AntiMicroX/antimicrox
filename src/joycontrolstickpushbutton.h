#ifndef JOYCONTROLSTICKPUSHBUTTON_H
#define JOYCONTROLSTICKPUSHBUTTON_H

#include "flashbuttonwidget.h"
#include "joycontrolstick.h"

class JoyControlStickPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyControlStickPushButton(JoyControlStick *stick, QWidget *parent = 0);
    JoyControlStick* getStick();
    
protected:
    virtual QString generateLabel();

    JoyControlStick *stick;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();
};

#endif // JOYCONTROLSTICKPUSHBUTTON_H
