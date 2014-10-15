#ifndef JOYBUTTONWIDGET_H
#define JOYBUTTONWIDGET_H

#include <QPoint>

#include "flashbuttonwidget.h"
#include "joybutton.h"

class JoyButtonWidget : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyButtonWidget(JoyButton* button, bool displayNames, QWidget *parent=0);
    JoyButton* getJoyButton();

protected:
    virtual QString generateLabel();

    JoyButton* button;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();

private slots:
    void showContextMenu(const QPoint &point);
};

#endif // JOYBUTTONWIDGET_H
