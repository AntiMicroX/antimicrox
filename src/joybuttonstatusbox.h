#ifndef JOYBUTTONSTATUSBOX_H
#define JOYBUTTONSTATUSBOX_H

#include <QPushButton>

#include "joybutton.h"

class JoyButtonStatusBox : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyButtonStatusBox(JoyButton *button, QWidget *parent = 0);
    JoyButton* getJoyButton();
    bool isButtonFlashing();

protected:
    JoyButton *button;
    bool isflashing;

signals:
    void flashed(bool flashing);

private slots:
    void flash();
    void unflash();
};

#endif // JOYBUTTONSTATUSBOX_H
