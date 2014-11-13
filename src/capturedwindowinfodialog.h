#ifndef UNIXWINDOWINFODIALOG_H
#define UNIXWINDOWINFODIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class CapturedWindowInfoDialog;
}

class CapturedWindowInfoDialog : public QDialog
{
    Q_OBJECT

public:

#ifdef Q_OS_WIN
    explicit CapturedWindowInfoDialog(QWidget *parent = 0);
#else
    explicit CapturedWindowInfoDialog(unsigned long window, QWidget *parent = 0);
#endif

    ~CapturedWindowInfoDialog();

    enum {
        WindowNone = 0,
        WindowClass = (1 << 0),
        WindowName = (1 << 1),
        WindowPath = (1 << 2),
    };
    typedef unsigned int CapturedWindowOption;

    QString getWindowClass();
    QString getWindowName();
    QString getWindowPath();
    bool useFullWindowPath();
    CapturedWindowOption getSelectedOptions();

private:
    Ui::CapturedWindowInfoDialog *ui;

protected:
    CapturedWindowOption selectedMatch;

    QString winClass;
    QString winName;
    QString winPath;
    bool fullWinPath;

private slots:
    void populateOption();
};

#endif // UNIXWINDOWINFODIALOG_H
