#ifndef BUTTONEDITDIALOGTWO_H
#define BUTTONEDITDIALOGTWO_H

#include <QDialog>

#include "joybutton.h"
#include "keyboard/virtualkeyboardmousewidget.h"
#include "advancebuttondialog.h"

namespace Ui {
class ButtonEditDialogTwo;
}

class ButtonEditDialogTwo : public QDialog
{
    Q_OBJECT
    
public:
    explicit ButtonEditDialogTwo(JoyButton *button, QWidget *parent = 0);
    ~ButtonEditDialogTwo();
    
protected:
    JoyButton *button;

private:
    Ui::ButtonEditDialogTwo *ui;

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
