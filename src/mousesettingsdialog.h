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

    QTime lastMouseStatUpdate;
    Ui::MouseSettingsDialog *ui;

public slots:
    void changeSensitivityStatus(int index);
    void changeSpringSectionStatus(int index);
    void changeMouseSpeedBoxStatus(int index);
    void changeWheelSpeedBoxStatus(int index);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    void changeSmoothingStatus(int index);
    void updateWheelVerticalSpeedLabel(int value);
    void updateWheelHorizontalSpeedLabel(int value);
    void changeSensitivityStatusForMouseMode(int index);

    virtual void changeMouseMode(int index) = 0;
    virtual void changeMouseCurve(int index) = 0;

protected slots:
    void updateMouseCursorStatusLabels(int mouseX, int mouseY, int elapsed);
    void updateMouseSpringStatusLabels(int coordX, int coordY);
};

#endif // MOUSESETTINGSDIALOG_H
