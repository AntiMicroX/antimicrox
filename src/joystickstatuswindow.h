#ifndef JOYSTICKSTATUSWINDOW_H
#define JOYSTICKSTATUSWINDOW_H

#include <QDialog>

#include "joystick.h"

namespace Ui {
class JoystickStatusWindow;
}

class JoystickStatusWindow : public QDialog
{
    Q_OBJECT

public:
    explicit JoystickStatusWindow(QWidget *parent = 0);
    explicit JoystickStatusWindow(Joystick *joystick, QWidget *parent = 0);
    ~JoystickStatusWindow();

protected:
    Joystick *joystick;

private:
    Ui::JoystickStatusWindow *ui;

private slots:
    void restoreButtonStates();
};

#endif // JOYSTICKSTATUSWINDOW_H
