#ifndef QKEYDISPLAYDIALOG_H
#define QKEYDISPLAYDIALOG_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class QKeyDisplayDialog;
}

class QKeyDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QKeyDisplayDialog(QWidget *parent = 0);
    ~QKeyDisplayDialog();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    Ui::QKeyDisplayDialog *ui;
};

#endif // QKEYDISPLAYDIALOG_H
