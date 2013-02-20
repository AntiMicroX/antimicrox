#ifndef AXISEDITDIALOG_H
#define AXISEDITDIALOG_H

#include <QDialog>

#include "joyaxis.h"
#include "axisvaluebox.h"
#include "buttontempconfig.h"

namespace Ui {
class AxisEditDialog;
}

class AxisEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AxisEditDialog(QWidget *parent = 0);
    AxisEditDialog(JoyAxis *axis, QWidget *parent=0);
    ~AxisEditDialog();

protected:
    JoyAxis *axis;
    ButtonTempConfig *tempPConfig;
    ButtonTempConfig *tempNConfig;

private:
    Ui::AxisEditDialog *ui;

private slots:
    void saveAxisChanges();
    void implementPresets(int index);
    void updateDeadZoneBox(int value);
    void updateMaxZoneBox(int value);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void updateThrottleUi(int index);
    void updateJoyValue(int index);
    void updateDeadZoneSlider(QString value);
    void updateMaxZoneSlider(QString value);
    void openAdvancedPDialog();
    void openAdvancedNDialog();
    void updateFromTempPConfig();
    void updateFromTempNConfig();
    void syncSpeedSpinBoxes();
    void moveSpeedsTogether(int value);
    void changeMouseSpeedsInterface(bool value);
    void updateTempConfigHorizontalSpeed(int value);
    void updateTempConfigVerticalSpeed(int value);
};

#endif // AXISEDITDIALOG_H
