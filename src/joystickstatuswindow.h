#ifndef JOYSTICKSTATUSWINDOW_H
#define JOYSTICKSTATUSWINDOW_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class JoystickStatusWindow;
}

class JoystickStatusWindow : public QDialog
{
    Q_OBJECT

public:
    explicit JoystickStatusWindow(InputDevice *joystick, QWidget *parent = 0);
    ~JoystickStatusWindow();

protected:
    InputDevice *joystick;

private:
    Ui::JoystickStatusWindow *ui;

private slots:
    void restoreButtonStates(int code);
    void obliterate();
};

#endif // JOYSTICKSTATUSWINDOW_H
