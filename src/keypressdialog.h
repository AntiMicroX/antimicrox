#ifndef KEYDELAYDIALOG_H
#define KEYDELAYDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class KeyPressDialog;
}

class KeyPressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyPressDialog(InputDevice *device, QWidget *parent = 0);
    ~KeyPressDialog();

protected:
    InputDevice *device;

private:
    Ui::KeyPressDialog *ui;

private slots:
    void changeDeviceKeyPress(int value);
};

#endif // KEYDELAYDIALOG_H
