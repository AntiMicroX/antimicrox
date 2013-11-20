#ifndef MOUSEBUTTONSETTINGSDIALOG_H
#define MOUSEBUTTONSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "springmoderegionpreview.h"

#include <joybutton.h>

class MouseButtonSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseButtonSettingsDialog(JoyButton *button, QWidget *parent = 0);
    
protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    void selectSmoothingPreset();

    JoyButton *button;
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

#endif // MOUSEBUTTONSETTINGSDIALOG_H
