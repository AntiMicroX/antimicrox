#ifndef WINAPPPROFILETIMERDIALOG_H
#define WINAPPPROFILETIMERDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class WinAppProfileTimerDialog;
}

class WinAppProfileTimerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WinAppProfileTimerDialog(QWidget *parent = 0);
    ~WinAppProfileTimerDialog();

protected:
    QTimer appTimer;

//slots:
//    void

private:
    Ui::WinAppProfileTimerDialog *ui;

private slots:
    void startTimer();
};

#endif // WINAPPPROFILETIMERDIALOG_H
