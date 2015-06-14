#ifndef MOUSESETTINGSDIALOG_H
#define MOUSESETTINGSDIALOG_H

#include <QDialog>
#include <QTime>

#include "joybutton.h"

namespace Ui {
class MouseSettingsDialog;
}

class MouseSettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MouseSettingsDialog(QWidget *parent = 0);
    ~MouseSettingsDialog();
    
protected:
    void updateAccelerationCurvePresetComboBox(JoyButton::JoyMouseCurve mouseCurve);
    JoyButton::JoyMouseCurve getMouseCurveForIndex(int index);

    Ui::MouseSettingsDialog *ui;
    QTime lastMouseStatUpdate;

public slots:
    void changeSettingsWidgetStatus(int index);
    void changeSpringSectionStatus(int index);
    void changeMouseSpeedBoxStatus(int index);
    void changeWheelSpeedBoxStatus(int index);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    //void changeSmoothingStatus(int index);
    void updateWheelVerticalSpeedLabel(int value);
    void updateWheelHorizontalSpeedLabel(int value);
    void changeSensitivityStatusForMouseMode(int index);

    virtual void changeMouseMode(int index) = 0;
    virtual void changeMouseCurve(int index) = 0;

private slots:
    void updateMouseCursorStatusLabels(int mouseX, int mouseY, int elapsed);
    void updateMouseSpringStatusLabels(int coordX, int coordY);
    void refreshMouseCursorSpeedValues(int index);
    void disableReleaseSpringBox(bool enable);
    void resetReleaseRadius(bool enabled);

    //void clampMinAccelThresholdValue(double value);
    //void clampMaxAccelThresholdValue(double value);
};

#endif // MOUSESETTINGSDIALOG_H
