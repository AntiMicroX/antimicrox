#ifndef DPADPUSHBUTTON_H
#define DPADPUSHBUTTON_H

#include "flashbuttonwidget.h"
#include "joydpad.h"

class DPadPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit DPadPushButton(JoyDPad *dpad, QWidget *parent = 0);
    JoyDPad* getDPad();

protected:
    QString generateLabel();

    JoyDPad *dpad;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();
};

#endif // DPADPUSHBUTTON_H
