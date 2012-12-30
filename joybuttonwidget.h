#ifndef JOYBUTTONWIDGET_H
#define JOYBUTTONWIDGET_H

#include <QPainter>
#include <QPushButton>

#include "joybutton.h"

class JoyButtonWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit JoyButtonWidget(QWidget *parent = 0);
    explicit JoyButtonWidget(JoyButton* button, QWidget *parent=0);
    JoyButton* getJoyButton();

protected:
    JoyButton* button;
    QPalette normal;
    QPalette flashing;
    bool isflashing;

signals:
    void flashed(bool flashing);

public slots:
    void changeKeyLabel();
    //void changeKeyLabel(QString label);
    void changeMouseLabel();
    //void changeMouseLabel(QString label);
    void refreshLabel();
    void disableFlashes();
    void enableFlashes();

private slots:
    void flash();
    void unflash();
};

#endif // JOYBUTTONWIDGET_H
