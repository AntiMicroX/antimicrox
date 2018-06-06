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

    int chooseMinMax(QString min_max_sign, QList<int> ax_values);
    bool ifGtkJstestRunToday();
    const QString getSetfromGtkJstest();


protected:
    void setProgressBars(int inputDevNr, int setJoyNr, int stickNr);
    void setProgressBars(JoyControlStick* controlstick);
    void updateAxesBox();
    bool enoughProb();
    int calculateRawVal(QHash<QString,int> ax_values, JoyAxis* joyAxis);
    void calibrate(JoyControlStick* stick);
    void setInfoText(int deadZoneX, int deadZoneY);


private:
    Ui::Calibration *ui;
    QMap<SDL_JoystickID, InputDevice*>* joysticks;
    JoyControlStick *stick;
    JoyControlStickEditDialogHelper helper;
    JoyAxis* joyAxisX;
    JoyAxis* joyAxisY;
    QProgressBar *axisBarX;
    QProgressBar *axisBarY;
    QMultiHash<QString,int> x_es_val;
    QMultiHash<QString,int> y_es_val;
    int sumX;
    int sumY;


public slots:
    void saveSettings();
    void checkX(int value);
    void checkY(int value);
    void createAxesConnection();
    void setController(QString controllerName);
    void loadSetFromJstest();
    void startCalibration();
    void startSecondStep();
    void startLastStep();

signals:
    void deadZoneChanged(int value);
    void propertyUpdated();


};

#endif // CALIBRATION_H
