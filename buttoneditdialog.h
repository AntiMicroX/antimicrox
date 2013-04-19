#ifndef BUTTONEDITDIALOG_H
#define BUTTONEDITDIALOG_H

#include <QDialog>

#include "joybutton.h"
#include "joydpadbutton.h"
#include "buttontempconfig.h"

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
    void updateTempConfigState();

    JoyButton* button;
    bool isEditing;
    QString defaultLabel;
    ButtonTempConfig *tempconfig;

private slots:
    void saveButtonChanges();
    void changeDialogText(bool edited=false);
    void disableDialogButtons();
    void enableDialogButtons();
    void openAdvancedDialog();
    void updateFromTempConfig();
    void singleAssignmentForTempConfig(bool edited);
};

#endif // BUTTONEDITDIALOG_H
