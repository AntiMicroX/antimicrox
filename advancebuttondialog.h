#ifndef ADVANCEBUTTONDIALOG_H
#define ADVANCEBUTTONDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "joybutton.h"
#include "buttontempconfig.h"
#include "simplekeygrabberbutton.h"

namespace Ui {
class AdvanceButtonDialog;
}

class AdvanceButtonDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AdvanceButtonDialog(JoyButton *button, QWidget *parent=0);
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
    JoyButton *button;

signals:
    void toggleChanged(bool state);
    void turboChanged(bool state);
    void slotsChanged();
    void turboButtonEnabledChange(bool state);

public slots:
    void placeNewSlot(JoyButtonSlot *slot);
    void clearAllSlots();

private slots:
    void changeTurboText(int value);
    void updateTurboIntervalValue(int value);
    void checkTurboSetting(bool state);

    void updateTempConfig();
    void updateSlotsScrollArea(int value);
    void deleteSlot();
    void changeSelectedSlot();
    void insertSlot();

    void insertPauseSlot();
    void insertHoldSlot();
    void insertCycleSlot();
    void insertDistanceSlot();

    void updateActionTimeLabel();
    void updateSetSelection();
};

Q_DECLARE_METATYPE(AdvanceButtonDialog*)

#endif // ADVANCEBUTTONDIALOG_H
