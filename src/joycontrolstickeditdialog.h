#ifndef JOYCONTROLSTICKEDITDIALOG_H
#define JOYCONTROLSTICKEDITDIALOG_H

#include <QDialog>

#include "joycontrolstick.h"

namespace Ui {
class JoyControlStickEditDialog;
}

class JoyControlStickEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent = 0);
    ~JoyControlStickEditDialog();
    
protected:
    void selectCurrentPreset();

    JoyControlStick *stick;

private:
    Ui::JoyControlStickEditDialog *ui;

private slots:
    void implementPresets(int index);
    void implementModes(int index);

    void refreshStickStats(int x, int y);
    void updateMouseMode(int index);

    void checkMaxZone(int value);
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
    void updateWindowTitleStickName();
};

#endif // JOYCONTROLSTICKEDITDIALOG_H
