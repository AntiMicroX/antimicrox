#ifndef VIRTUALDPADPUSHBUTTON_H
#define VIRTUALDPADPUSHBUTTON_H

#include <QPushButton>
#include <QPaintEvent>

#include "vdpad.h"

class VirtualDPadPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit VirtualDPadPushButton(VDPad *vdpad, QWidget *parent = 0);
    VDPad* getVDPad();
    bool isButtonFlashing();

protected:
    virtual void paintEvent(QPaintEvent *event);
    QString generateLabel();

    VDPad *vdpad;
    bool isflashing;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void disableFlashes();
    void enableFlashes();

private slots:
    void flash();
    void unflash();
};

#endif // VIRTUALDPADPUSHBUTTON_H
