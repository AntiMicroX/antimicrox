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
    void selectCurrentPreset();

    JoyDPad *dpad;

private:
    Ui::DPadEditDialog *ui;

private slots:
    void implementPresets(int index);
    void implementModes(int index);
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
};

#endif // DPADEDITDIALOG_H
