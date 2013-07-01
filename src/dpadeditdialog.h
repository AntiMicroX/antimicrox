#ifndef DPADEDITDIALOG_H
#define DPADEDITDIALOG_H

#include <QDialog>

#include "joydpad.h"

namespace Ui {
class DPadEditDialog;
}

class DPadEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DPadEditDialog(JoyDPad *dpad, QWidget *parent = 0);
    ~DPadEditDialog();

protected:
    JoyDPad *dpad;

private:
    Ui::DPadEditDialog *ui;

private slots:
    void implementPresets(int index);
    void implementModes(int index);

    void syncSpeedSpinBoxes();
    void changeMouseSpeedsInterface(bool value);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
};

#endif // DPADEDITDIALOG_H
