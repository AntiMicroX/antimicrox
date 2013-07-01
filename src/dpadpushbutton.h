#ifndef DPADPUSHBUTTON_H
#define DPADPUSHBUTTON_H

#include <QPushButton>
#include <QPaintEvent>

#include "joydpad.h"

class DPadPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit DPadPushButton(JoyDPad *dpad, QWidget *parent = 0);
    JoyDPad* getDPad();
    bool isButtonFlashing();

protected:
    virtual void paintEvent(QPaintEvent *event);
    QString generateLabel();

    JoyDPad *dpad;
    bool isflashing;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void unflash();
    void disableFlashes();
    void enableFlashes();

private slots:
    void flash();
};

#endif // DPADPUSHBUTTON_H
