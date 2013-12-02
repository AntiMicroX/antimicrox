#ifndef ADVANCESTICKASSIGNMENTDIALOG_H
#define ADVANCESTICKASSIGNMENTDIALOG_H

#include <QDialog>

#include "joystick.h"

namespace Ui {
class AdvanceStickAssignmentDialog;
}

class AdvanceStickAssignmentDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AdvanceStickAssignmentDialog(Joystick *joystick, QWidget *parent = 0);
    ~AdvanceStickAssignmentDialog();
    
protected:
    Joystick *joystick;

signals:
    void stickConfigurationChanged();
    void vdpadConfigurationChanged();

private:
    Ui::AdvanceStickAssignmentDialog *ui;

private slots:
    void refreshStickConfiguration();
    void refreshVDPadConfiguration();

    void checkForAxisAssignmentStickOne();
    void checkForAxisAssignmentStickTwo();

    void changeStateStickOneWidgets(bool enabled);
    void changeStateStickTwoWidgets(bool enabled);
    void changeStateVDPadWidgets(bool enabled);

    void populateDPadComboBoxes();

    void changeVDPadUpButton(int index);
    void changeVDPadDownButton(int index);
    void changeVDPadLeftButton(int index);
    void changeVDPadRightButton(int index);

    void disableVDPadComboBoxes();
    void enableVDPadComboBoxes();

    void openQuickAssignDialogStick1();
    void openQuickAssignDialogStick2();

    void quickAssignStick1Axis1();
    void quickAssignStick1Axis2();

    void quickAssignStick2Axis1();
    void quickAssignStick2Axis2();

    void openAssignVDPadUp();
    void openAssignVDPadDown();
    void openAssignVDPadLeft();
    void openAssignVDPadRight();

    void quickAssignVDPadUp();
    void quickAssignVDPadDown();
    void quickAssignVDPadLeft();
    void quickAssignVDPadRight();

    void reenableButtonEvents();
};

#endif // ADVANCESTICKASSIGNMENTDIALOG_H
