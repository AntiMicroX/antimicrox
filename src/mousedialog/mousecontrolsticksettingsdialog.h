#ifndef MOUSECONTROLSTICKSETTINGSDIALOG_H
#define MOUSECONTROLSTICKSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "joycontrolstick.h"

class MouseControlStickSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent=0);
    //explicit MouseControlStickSettingsDialog(QWidget *parent = 0);
    
protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();

    JoyControlStick *stick;

signals:
    
public slots:
    void changeMouseMode(int index);
    void changeMouseCurve(int index);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
    void updateSpringWidth(int value);
    void updateSpringHeight(int value);
    void updateSensitivity(double value);
};

#endif // MOUSECONTROLSTICKSETTINGSDIALOG_H
