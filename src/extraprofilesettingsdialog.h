#ifndef KEYDELAYDIALOG_H
#define KEYDELAYDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class ExtraProfileSettingsDialog;
}

class ExtraProfileSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtraProfileSettingsDialog(InputDevice *device, QWidget *parent = 0);
    ~ExtraProfileSettingsDialog();

protected:
    InputDevice *device;

private:
    Ui::ExtraProfileSettingsDialog *ui;

private slots:
    void changeDeviceKeyPress(int value);
};

#endif // KEYDELAYDIALOG_H
