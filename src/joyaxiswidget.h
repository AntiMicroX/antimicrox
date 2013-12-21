#ifndef JOYAXISWIDGET_H
#define JOYAXISWIDGET_H

#include "flashbuttonwidget.h"
#include "joyaxis.h"

class JoyAxisWidget : public FlashButtonWidget
{
    Q_OBJECT

public:
    explicit JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent=0);
    JoyAxis* getAxis();

protected:
    virtual QString generateLabel();

    JoyAxis *axis;

signals:

public slots:
    void disableFlashes();
    void enableFlashes();
};

#endif // JOYAXISWIDGET_H
