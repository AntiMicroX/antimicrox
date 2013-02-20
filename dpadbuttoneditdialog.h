#ifndef DPADBUTTONEDITDIALOG_H
#define DPADBUTTONEDITDIALOG_H

#include <QDialog>

#include "joydpadbutton.h"
#include "buttoneditdialog.h"

namespace Ui {
class DPadButtonEditDialog;
}

class DPadButtonEditDialog : public ButtonEditDialog
{
    Q_OBJECT
    
public:
    explicit DPadButtonEditDialog(JoyDPadButton *button, QWidget *parent=0);
    //~DPadButtonEditDialog();

protected:
    //JoyDPadButton* button;
    //bool isEditing;
    //int pendingMouseCode;

private:
    //Ui::DPadButtonEditDialog *ui;

private slots:
    //void saveButtonChanges();
    //void changeDialogText(bool edited=false);
   // void changeMouseSetting(int mousecode);

};

#endif // DPADBUTTONEDITDIALOG_H
