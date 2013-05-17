#ifndef BUTTONEDITDIALOGTWO_H
#define BUTTONEDITDIALOGTWO_H

#include <QDialog>

#include "joybutton.h"
#include "keyboard/virtualkeyboardmousewidget.h"
#include "advancebuttondialog.h"

namespace Ui {
class ButtonEditDialog;
}

class ButtonEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ButtonEditDialog(JoyButton *button, QWidget *parent = 0);
    ~ButtonEditDialog();
    
protected:
    JoyButton *button;

private:
    Ui::ButtonEditDialog *ui;

signals:
    void advancedDialogOpened();
    void sendTempSlotToAdvanced(JoyButtonSlot *tempslot);

private slots:
    void refreshSlotSummaryLabel();
    void changeToggleSetting();
    void changeTurboSetting();
    void openAdvancedDialog();
    void createTempSlot(int keycode);

    void checkTurboSetting(bool state);
    void setTurboButtonEnabled(bool state);
};

#endif // BUTTONEDITDIALOGTWO_H
