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

    JoyAxis *axis;
    SetAxisThrottleDialog *setAxisThrottleConfirm;
    int initialThrottleState;

private:
    Ui::AxisEditDialog *ui;

private slots:
    void implementPresets(int index);
    void updateDeadZoneBox(int value);
    void updateMaxZoneBox(int value);
    void updateThrottleUi(int index);
    void updateJoyValue(int value);
    void updateDeadZoneSlider(QString value);
    void updateMaxZoneSlider(QString value);
    void openAdvancedPDialog();
    void openAdvancedNDialog();

    void refreshPButtonLabel();
    void refreshNButtonLabel();
    void refreshPreset();

    void checkFinalSettings();
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
};

#endif // AXISEDITDIALOG_H
