#ifndef JOYDPADBUTTONWIDGET_H
#define JOYDPADBUTTONWIDGET_H

#include "joybuttonwidget.h"

class JoyDPadButtonWidget : public JoyButtonWidget
{
    Q_OBJECT
public:
    explicit JoyDPadButtonWidget(JoyButton* button, QWidget *parent = 0);

protected:
    QString generateLabel();

signals:
    
public slots:
    virtual void refreshLabel();
};

#endif // JOYDPADBUTTONWIDGET_H
