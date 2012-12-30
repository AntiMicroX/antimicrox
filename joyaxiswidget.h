#ifndef JOYAXISWIDGET_H
#define JOYAXISWIDGET_H

#include <QPushButton>

#include "joyaxis.h"

class JoyAxisWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit JoyAxisWidget(QWidget *parent = 0);
    explicit JoyAxisWidget(JoyAxis *axis, QWidget *parent=0);
    JoyAxis* getAxis();

protected:
    JoyAxis *axis;
    QPalette normal;
    QPalette flashing;
    bool isflashing;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void unflash();
    void disableFlashes();
    void enableFlashes();

private slots:
    void flash();
};

#endif // JOYAXISWIDGET_H
