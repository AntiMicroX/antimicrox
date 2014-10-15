#ifndef DPADPUSHBUTTON_H
#define DPADPUSHBUTTON_H

#include <QPoint>

#include "flashbuttonwidget.h"
#include "joydpad.h"

class DPadPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent = 0);
    JoyDPad* getDPad();

protected:
    QString generateLabel();

    JoyDPad *dpad;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();

private slots:
    void showContextMenu(const QPoint &point);
};

#endif // DPADPUSHBUTTON_H
