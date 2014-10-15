#ifndef JOYCONTROLSTICKPUSHBUTTON_H
#define JOYCONTROLSTICKPUSHBUTTON_H

#include <QPoint>

#include "flashbuttonwidget.h"
#include "joycontrolstick.h"

class JoyControlStickPushButton : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyControlStickPushButton(JoyControlStick *stick, bool displayNames, QWidget *parent = 0);
    JoyControlStick* getStick();
    
protected:
    virtual QString generateLabel();

    JoyControlStick *stick;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();

private slots:
    void showContextMenu(const QPoint &point);
};

#endif // JOYCONTROLSTICKPUSHBUTTON_H
