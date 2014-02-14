#ifndef SETNAMESDIALOG_H
#define SETNAMESDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class SetNamesDialog;
}

class SetNamesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetNamesDialog(InputDevice *device, QWidget *parent = 0);
    ~SetNamesDialog();

protected:
    InputDevice *device;

private:
    Ui::SetNamesDialog *ui;

private slots:
    void saveSetNameChanges();
};

#endif // SETNAMESDIALOG_H
