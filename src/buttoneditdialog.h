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
    bool ignoreRelease;

    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::ButtonEditDialog *ui;

signals:
    void advancedDialogOpened();
    void sendTempSlotToAdvanced(JoyButtonSlot *tempslot);
    void keyGrabbed(JoyButtonSlot *tempslot);
    void selectionCleared();
    void selectionFinished();

private slots:
    void refreshSlotSummaryLabel();
    void changeToggleSetting();
    void changeTurboSetting();
    void openAdvancedDialog();
    void closedAdvancedDialog();
    void createTempSlot(int keycode);

    void checkTurboSetting(bool state);
    void setTurboButtonEnabled(bool state);
    void processSlotAssignment(JoyButtonSlot *tempslot);
    void clearButtonSlots();
    void sendSelectionFinished();
    void updateWindowTitleButtonName();
    void checkForKeyboardWidgetFocus(QWidget *old, QWidget *now);
};

#endif // BUTTONEDITDIALOGTWO_H
