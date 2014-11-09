#ifndef UNIXWINDOWINFODIALOG_H
#define UNIXWINDOWINFODIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class UnixWindowInfoDialog;
}

class UnixWindowInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnixWindowInfoDialog(unsigned long window, QWidget *parent = 0);
    ~UnixWindowInfoDialog();

    enum DialogWindowOption {
        WindowClass=0,
        WindowName,
        WindowPath
    };

    QString getPropertyValue();
    DialogWindowOption getSelectedOption();

private:
    Ui::UnixWindowInfoDialog *ui;

protected:
    DialogWindowOption selectedMatch;
    QString selectedValue;

    QString winClass;
    QString winName;
    QString winPath;

private slots:
    void populateOption();
};

#endif // UNIXWINDOWINFODIALOG_H
