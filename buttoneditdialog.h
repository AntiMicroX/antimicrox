#ifndef BUTTONEDITDIALOG_H
#define BUTTONEDITDIALOG_H

#include <QDialog>

#include "joybutton.h"
#include "joydpadbutton.h"

namespace Ui {
class ButtonEditDialog;
}

class ButtonEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ButtonEditDialog(JoyButton* button, QWidget *parent=0);
    ~ButtonEditDialog();

private:
    Ui::ButtonEditDialog *ui;

protected:
    JoyButton* button;
    bool isEditing;
    QString defaultLabel;

signals:

public slots:


private slots:
    void saveButtonChanges();
    void changeDialogText(bool edited=false);
    void changeTurboText(int value);
    void disableDialogButtons();
    void enableDialogButtons();
    //void enableTurboSlider(bool enable);
};

#endif // BUTTONEDITDIALOG_H
