#ifndef AXISEDITDIALOG_H
#define AXISEDITDIALOG_H

#include <QDialog>

#include "joyaxis.h"

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

private:
    Ui::AxisEditDialog *ui;

private slots:
    void saveAxisChanges();
    void updateModeUi(int index);
    void updateDeadZoneBox(int value);
    void updateMaxZoneBox(int value);
    void updateSpeedConvertLabel(int value);
    void updateThrottleUi(int index);
};

#endif // AXISEDITDIALOG_H
