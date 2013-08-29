#ifndef AXISEDITDIALOG_H
#define AXISEDITDIALOG_H

#include <QDialog>

#include "joyaxis.h"
#include "axisvaluebox.h"
#include "setaxisthrottledialog.h"

namespace Ui {
class AxisEditDialog;
}

class AxisEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AxisEditDialog(JoyAxis *axis, QWidget *parent=0);
    ~AxisEditDialog();

protected:
    void selectCurrentPreset();
    void selectCurrentMouseModePreset();

    JoyAxis *axis;
    SetAxisThrottleDialog *setAxisThrottleConfirm;
    int initialThrottleState;

private:
    Ui::AxisEditDialog *ui;

private slots:
    void implementPresets(int index);
    void updateDeadZoneBox(int value);
    void updateMaxZoneBox(int value);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void updateThrottleUi(int index);
    void updateJoyValue(int value);
    void updateDeadZoneSlider(QString value);
    void updateMaxZoneSlider(QString value);
    void openAdvancedPDialog();
    void openAdvancedNDialog();

    void refreshPButtonLabel();
    void refreshNButtonLabel();
    void refreshPreset();

    void syncSpeedSpinBoxes();
    void moveSpeedsTogether(int value);
    void changeMouseSpeedsInterface(bool value);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
    void updateMouseMode(int index);

    void checkFinalSettings();
};

#endif // AXISEDITDIALOG_H
