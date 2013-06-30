#ifndef JOYAXISWIDGET_H
#define JOYAXISWIDGET_H

#include <QPushButton>
#include <QPaintEvent>

#include "joyaxis.h"

class JoyAxisWidget : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyAxisWidget(JoyAxis *axis, QWidget *parent=0);
    JoyAxis* getAxis();
    bool isButtonFlashing();

protected:
    virtual void paintEvent(QPaintEvent *event);
    QString generateLabel();

    JoyAxis *axis;
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

#endif // JOYAXISWIDGET_H
