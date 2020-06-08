/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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


#include "uihelpers/advancebuttondialoghelper.h"

#include <QDialog>
#include <QReadWriteLock>

class JoyButton;
class SimpleKeyGrabberButton;
class QListWidgetItem;

namespace Ui {
class AdvanceButtonDialog;
}

class AdvanceButtonDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AdvanceButtonDialog(JoyButton *button, QWidget *parent=0);
    AdvanceButtonDialog(QWidget *parent=0);
    ~AdvanceButtonDialog();

    int getOldRow() const;
    JoyButton *getButton() const;
    AdvanceButtonDialogHelper const& getHelper();

protected:
    void connectButtonEvents(SimpleKeyGrabberButton *button); // AdvanceBtnDlgAssign class
    void appendBlankKeyGrabber(); // AdvanceBtnDlgAssign class
    int actionTimeConvert(); // AdvanceBtnDlgAssign class
    void changeTurboForSequences();
    void fillTimeComboBoxes(); // AdvanceBtnDlgAssign class
    void refreshTimeComboBoxes(JoyButtonSlot *slot); // AdvanceBtnDlgAssign class
    void updateWindowTitleButtonName(); // AdvanceBtnDlgAssign class
    void populateAutoResetInterval(); // AdvanceBtnDlgAssign class
    void disconnectTimeBoxesEvents(); // AdvanceBtnDlgAssign class
    void connectTimeBoxesEvents(); // AdvanceBtnDlgAssign class
    void resetTimeBoxes(); // AdvanceBtnDlgAssign class
    void populateSetSelectionComboBox();
    void populateSlotSetSelectionComboBox();
    void findTurboModeComboIndex();
    void showSelectProfileWind(QListWidgetItem* item, QString& firstChoiceProfile);
    bool anySelectedNotKeybSlot();
    bool selectedNotMixSlot();

signals:
    void toggleChanged(bool state);
    void turboChanged(bool state);
    void slotsChanged(); // AdvanceBtnDlgAssign class
    void turboButtonEnabledChange(bool state);

public slots:
    void placeNewSlot(JoyButtonSlot *slot); // AdvanceBtnDlgAssign class
    void clearAllSlots(); // AdvanceBtnDlgAssign class

private slots:
    void changeTurboText(int value);
    void updateTurboIntervalValue(int value);
    void checkTurboSetting(bool state);

    void updateSlotsScrollArea(int value); // AdvanceBtnDlgAssign class
    void deleteSlot(bool showWarning = true);  // AdvanceBtnDlgAssign class
    void insertSlot(); // AdvanceBtnDlgAssign class
    void joinSlot(); // AdvanceBtnDlgAssign class
    void splitSlot(); // AdvanceBtnDlgAssign class

    void insertCycleSlot(QListWidgetItem* item); // AdvanceBtnDlgAssign class
    void insertTextEntrySlot(QListWidgetItem* item); // AdvanceBtnDlgAssign class
    void insertExecuteSlot(QListWidgetItem* item, QStringList& prevExecAndArgs); // AdvanceBtnDlgAssign class

    void updateActionTimeLabel();
    void updateSetSelection();
    void checkTurboIntervalValue(int value);
    void performStatsWidgetRefresh(QListWidgetItem *item);

    void checkSlotTimeUpdate(); // AdvanceBtnDlgAssign class
    void checkSlotMouseModUpdate(); // AdvanceBtnDlgAssign class
    void checkSlotDistanceUpdate(); // AdvanceBtnDlgAssign class
    void checkSlotSetChangeUpdate(); // AdvanceBtnDlgAssign class

    void checkCycleResetWidgetStatus(bool enabled); // AdvanceBtnDlgAssign class
    void setButtonCycleResetInterval(double value); // AdvanceBtnDlgAssign class
    void setButtonCycleReset(bool enabled); // AdvanceBtnDlgAssign class
    void setButtonTurboMode(int value);
    void showSelectProfileWindow();
    void showFindExecutableWindow(bool); // AdvanceBtnDlgAssign class

    void changeSlotTypeDisplay(int index); // AdvanceBtnDlgAssign class
    void changeSlotHelpText(int index); // AdvanceBtnDlgAssign class

private:

    Ui::AdvanceButtonDialog *ui;

    AdvanceButtonDialogHelper& getHelperLocal();

    enum SlotTypeComboIndex {
        KBMouseSlot = 0, CycleSlot, DelaySlot, DistanceSlot, ExecuteSlot,
        HoldSlot, LoadSlot, MouseModSlot, PauseSlot, PressTimeSlot,
        ReleaseSlot, SetChangeSlot, TextEntry
    };

    int oldRow;
    JoyButton *m_button;
    AdvanceButtonDialogHelper helper;
    QReadWriteLock joinLock;

    void insertKindOfSlot(QListWidgetItem* item, int slotProperty, JoyButtonSlot::JoySlotInputAction inputAction); // AdvanceBtnDlgAssign class

};

#endif // ADVANCEBUTTONDIALOG_H
