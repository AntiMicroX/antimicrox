#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "joycontrolstickeditdialog.h"

#include <SDL2/SDL_joystick.h>

#include <QWidget>

class JoyControlStick;
class InputDevice;

namespace Ui {
class Calibration;
}

class Calibration : public QWidget
{
    Q_OBJECT

public:
    explicit Calibration(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent = 0);
    ~Calibration();

private:
    Ui::Calibration *ui;
    QMap<SDL_JoystickID, InputDevice*>* joysticks;
    JoyControlStick *stick;
    JoyControlStickEditDialogHelper helper;

public slots:
    void saveSettings();
    void checkX();
    void checkY();
    void createAxesConnection();
    void setController(QString controllerName);
    void loadSetFromJstest();
};

#endif // CALIBRATION_H
