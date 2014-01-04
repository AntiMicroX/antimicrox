#ifndef MOUSECONTROLSTICKSETTINGSDIALOG_H
#define MOUSECONTROLSTICKSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "springmoderegionpreview.h"

#include <joycontrolstick.h>

class MouseControlStickSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent=0);
    
protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    void selectSmoothingPreset();
    void calculateWheelSpeedPreset();
    void updateWindowTitleStickName();

    JoyControlStick *stick;
    SpringModeRegionPreview *springPreviewWidget;

signals:
    
public slots:
    void changeMouseMode(int index);
    void changeMouseCurve(int index);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
    void updateSpringWidth(int value);
    void updateSpringHeight(int value);
    void updateSensitivity(double value);
    void updateAccelerationCurvePresetComboBox();
    void updateSmoothingSetting(bool clicked);
    void updateWheelSpeedHorizontalSpeed(int value);
    void updateWheelSpeedVerticalSpeed(int value);
};

#endif // MOUSECONTROLSTICKSETTINGSDIALOG_H
