#ifndef JOYDPADBUTTONWIDGET_H
#define JOYDPADBUTTONWIDGET_H

#include "joybuttonwidget.h"

class JoyDPadButtonWidget : public JoyButtonWidget
{
    Q_OBJECT
public:
    explicit JoyDPadButtonWidget(JoyButton* button, QWidget *parent = 0);

protected:
    virtual QString generateLabel();

signals:
    
public slots:
};

#endif // JOYDPADBUTTONWIDGET_H
