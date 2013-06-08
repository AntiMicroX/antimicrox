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
    void refreshStickConfiguration();

    Joystick *joystick;

private:
    Ui::AdvanceStickAssignmentDialog *ui;

private slots:
    void checkForAxisAssignmentStickOne();
    void checkForAxisAssignmentStickTwo();

    void changeStateStickOneWidgets(bool enabled);
    void changeStateStickTwoWidgets(bool enabled);
};

#endif // ADVANCESTICKASSIGNMENTDIALOG_H
