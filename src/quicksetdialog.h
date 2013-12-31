#ifndef QUICKSETDIALOG_H
#define QUICKSETDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class QuickSetDialog;
}

class QuickSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickSetDialog(InputDevice *joystick, QWidget *parent = 0);
    ~QuickSetDialog();

protected:
    InputDevice *joystick;
    QDialog *currentButtonDialog;

private:
    Ui::QuickSetDialog *ui;

signals:
    void buttonDialogClosed();

private slots:
    void showAxisButtonDialog();
    void showButtonDialog();
    void showStickButtonDialog();
    void showDPadButtonDialog();

    void nullifyDialogPointer();
    void restoreButtonStates();
};

#endif // QUICKSETDIALOG_H
