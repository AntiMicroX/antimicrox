#ifndef JOYCONTROLSTICKBUTTONPUSHBUTTON_H
#define JOYCONTROLSTICKBUTTONPUSHBUTTON_H

#include "flashbuttonwidget.h"
#include "joycontrolstickbutton.h"

class JoyControlStickButtonPushButton : public FlashButtonWidget
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyControlStickButtonPushButton(JoyControlStickButton *button, QWidget *parent = 0);

    JoyControlStickButton* getButton();
    void setButton(JoyControlStickButton *button);

protected:
    virtual QString generateLabel();

    JoyControlStickButton *button;
    
signals:

public slots:
    void disableFlashes();
    void enableFlashes();
};

#endif // JOYCONTROLSTICKBUTTONPUSHBUTTON_H
