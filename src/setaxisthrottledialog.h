#ifndef SETAXISTHROTTLEDIALOG_H
#define SETAXISTHROTTLEDIALOG_H

#include <QDialog>

#include "joyaxis.h"

namespace Ui {
class SetAxisThrottleDialog;
}

class SetAxisThrottleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SetAxisThrottleDialog(JoyAxis *axis, QWidget *parent = 0);
    ~SetAxisThrottleDialog();
    
private:
    Ui::SetAxisThrottleDialog *ui;

protected:
    JoyAxis *axis;

signals:
    void initiateSetAxisThrottleChange();

private slots:
    void propogateThrottleChange();

};

#endif // SETAXISTHROTTLEDIALOG_H
