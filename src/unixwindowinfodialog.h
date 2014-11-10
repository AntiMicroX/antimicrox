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

    enum {
        WindowClass = (1 << 0),
        WindowName = (1 << 1),
        WindowPath = (1 << 2)
    };
    typedef unsigned int DialogWindowOption;

    QString getWindowClass();
    QString getWindowName();
    QString getWindowPath();
    DialogWindowOption getSelectedOptions();

private:
    Ui::UnixWindowInfoDialog *ui;

protected:
    DialogWindowOption selectedMatch;

    QString winClass;
    QString winName;
    QString winPath;

private slots:
    void populateOption();
};

#endif // UNIXWINDOWINFODIALOG_H
