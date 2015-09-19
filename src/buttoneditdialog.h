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

#ifndef BUTTONEDITDIALOGTWO_H
#define BUTTONEDITDIALOGTWO_H

#include <QDialog>

#include "joybutton.h"
#include "keyboard/virtualkeyboardmousewidget.h"
#include "advancebuttondialog.h"
#include "uihelpers/buttoneditdialoghelper.h"


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
    ButtonEditDialogHelper helper;

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
    void createTempSlot(int keycode, unsigned int alias);

    void checkTurboSetting(bool state);
    void setTurboButtonEnabled(bool state);
    void processSlotAssignment(JoyButtonSlot *tempslot);
    void clearButtonSlots();
    void sendSelectionFinished();
    void updateWindowTitleButtonName();
    void checkForKeyboardWidgetFocus(QWidget *old, QWidget *now);
};

#endif // BUTTONEDITDIALOGTWO_H
