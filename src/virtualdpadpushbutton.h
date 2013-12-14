#ifndef VIRTUALDPADPUSHBUTTON_H
#define VIRTUALDPADPUSHBUTTON_H

#include "flashbuttonwidget.h"
#include "vdpad.h"

class VirtualDPadPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit VirtualDPadPushButton(VDPad *vdpad, QWidget *parent = 0);
    VDPad* getVDPad();

protected:
    virtual QString generateLabel();

    VDPad *vdpad;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();
};

#endif // VIRTUALDPADPUSHBUTTON_H
