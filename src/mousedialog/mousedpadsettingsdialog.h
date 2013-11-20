#ifndef MOUSEDPADSETTINGSDIALOG_H
#define MOUSEDPADSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "springmoderegionpreview.h"

#include <joydpad.h>

class MouseDPadSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseDPadSettingsDialog(JoyDPad *dpad, QWidget *parent = 0);

protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    void selectSmoothingPreset();

    JoyDPad *dpad;
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
};

#endif // MOUSEDPADSETTINGSDIALOG_H
