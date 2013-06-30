#ifndef JOYBUTTONWIDGET_H
#define JOYBUTTONWIDGET_H

#include <QPushButton>
#include <QPaintEvent>

#include "joybutton.h"

class JoyButtonWidget : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyButtonWidget(JoyButton* button, QWidget *parent=0);
    JoyButton* getJoyButton();
    bool isButtonFlashing();

protected:
    virtual void paintEvent(QPaintEvent *event);
    QString generateLabel();

    JoyButton* button;
    bool isflashing;

signals:
    void flashed(bool flashing);

public slots:
    virtual void refreshLabel();
    void disableFlashes();
    void enableFlashes();

private slots:
    void flash();
    void unflash();
};

#endif // JOYBUTTONWIDGET_H
