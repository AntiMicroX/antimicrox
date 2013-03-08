#ifndef ADVANCEBUTTONDIALOG_H
#define ADVANCEBUTTONDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "buttontempconfig.h"
#include "simplekeygrabberbutton.h"

namespace Ui {
class AdvanceButtonDialog;
}

class AdvanceButtonDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AdvanceButtonDialog(ButtonTempConfig *tempconfig, QWidget *parent=0);
    ~AdvanceButtonDialog();

private:
    Ui::AdvanceButtonDialog *ui;

protected:
    void connectButtonEvents(SimpleKeyGrabberButton *button);
    void appendBlankKeyGrabber();
    int actionTimeConvert();
    void changeTurboForSequences();

    ButtonTempConfig *tempconfig;
    int oldRow;

private slots:
    void changeTurboText(int value);
    void updateTempConfig();
    void updateSlotsScrollArea(int value);
    void deleteSlot();
    void changeSelectedSlot();
    void updateHorizSpeedLabel(int value);
    void updateVertiSpeedLabel(int value);
    void addUpMovementSlot();
    void addDownMovementSlot();
    void addLeftMovementSlot();
    void addRightMovementSlot();
    void moveSpeedsTogether(int value);
    void syncSpeedSpinBoxes();
    void insertSlot();

    void insertPauseSlot();
    void insertHoldSlot();

    void updateActionTimeLabel();
    void clearAllSlots();
};

#endif // ADVANCEBUTTONDIALOG_H
