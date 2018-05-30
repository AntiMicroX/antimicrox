#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "joycontrolstickeditdialog.h"

#include <SDL2/SDL_joystick.h>

#include <QWidget>
#include <QProgressBar>
#include <QList>
#include <QHash>

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


protected:
    void setProgressBars(int inputDevNr, int setJoyNr, int stickNr);
    void setProgressBars(JoyControlStick* controlstick);
    void updateAxesBox();
    bool enoughProb();
    int calculateDeadZone(QHash<QString,int> ax_values);


private:
    Ui::Calibration *ui;
    QMap<SDL_JoystickID, InputDevice*>* joysticks;
    JoyControlStick *stick;
    JoyControlStickEditDialogHelper helper;
    JoyAxis* joyAxisX;
    JoyAxis* joyAxisY;
    QProgressBar *axisBarX;
    QProgressBar *axisBarY;
    QHash<QString,int> x_es_val;
    QHash<QString,int> y_es_val;


public slots:
    void saveSettings();
    void checkX(int value);
    void checkY(int value);
    void createAxesConnection();
    void setController(QString controllerName);
    void loadSetFromJstest();


};

#endif // CALIBRATION_H
