/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADVANCEBUTTONDIALOG_H
#define ADVANCEBUTTONDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "joybutton.h"
#include "simplekeygrabberbutton.h"
#include "uihelpers/advancebuttondialoghelper.h"

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

    enum SlotTypeComboIndex {
        KBMouseSlot = 0, CycleSlot, DelaySlot, DistanceSlot, ExecuteSlot,
        HoldSlot, LoadSlot, MouseModSlot, PauseSlot, PressTimeSlot,
        ReleaseSlot, SetChangeSlot, TextEntry,
    };

protected:
    void connectButtonEvents(SimpleKeyGrabberButton *button);
    void appendBlankKeyGrabber();
    int actionTimeConvert();
    void changeTurboForSequences();
    void fillTimeComboBoxes();
    void refreshTimeComboBoxes(JoyButtonSlot *slot);
    void updateWindowTitleButtonName();
    void populateAutoResetInterval();
    void disconnectTimeBoxesEvents();
    void connectTimeBoxesEvents();
    void resetTimeBoxes();
    void populateSetSelectionComboBox();
    void populateSlotSetSelectionComboBox();
    void findTurboModeComboIndex();

    int oldRow;
    JoyButton *button;
    AdvanceButtonDialogHelper helper;
    static const int MINIMUMTURBO;

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

    void updateSlotsScrollArea(int value);
    void deleteSlot();
    void changeSelectedSlot();
    void insertSlot();
    void updateSelectedSlot(int value);

    void insertPauseSlot();
    void insertHoldSlot();
    void insertCycleSlot();
    void insertDistanceSlot();
    void insertReleaseSlot();
    void insertMouseSpeedModSlot();
    void insertKeyPressSlot();
    void insertDelaySlot();
    void insertSetChangeSlot();
    void insertTextEntrySlot();
    void insertExecuteSlot();

    void updateActionTimeLabel();
    void updateSetSelection();
    void checkTurboIntervalValue(int value);
    void performStatsWidgetRefresh(QListWidgetItem *item);

    void checkSlotTimeUpdate();
    void checkSlotMouseModUpdate();
    void checkSlotDistanceUpdate();
    void checkSlotSetChangeUpdate();

    void checkCycleResetWidgetStatus(bool enabled);
    void setButtonCycleResetInterval(double value);
    void setButtonCycleReset(bool enabled);
    void setButtonTurboMode(int value);
    void showSelectProfileWindow();
    void showFindExecutableWindow(bool);

    void changeSlotTypeDisplay(int index);
    void changeSlotHelpText(int index);
};

#endif // ADVANCEBUTTONDIALOG_H
