#ifndef KEYDELAYDIALOG_H
#define KEYDELAYDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class KeyDelayDialog;
}

class KeyDelayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyDelayDialog(InputDevice *device, QWidget *parent = 0);
    ~KeyDelayDialog();

protected:
    InputDevice *device;

private:
    Ui::KeyDelayDialog *ui;

private slots:
    void changeDeviceKeyDelay(int value);
};

#endif // KEYDELAYDIALOG_H
