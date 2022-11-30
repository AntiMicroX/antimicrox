/* antimicrox Gamepad to KB+M event mapper
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

#ifndef BUTTONEDITDIALOGTWO_H
#define BUTTONEDITDIALOGTWO_H

#include <QDialog>

#include "uihelpers/buttoneditdialoghelper.h"

class JoyButton;
class JoyButtonSlot;
class QWidget;
class QKeyEvent;
class InputDevice;
class QuickSetDialog;

namespace Ui {
class ButtonEditDialog;
}

class ButtonEditDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ButtonEditDialog(JoyButton *button, InputDevice *joystick, bool isNumKeypad,
                              QWidget *parent = nullptr); // called for chosen button
    explicit ButtonEditDialog(InputDevice *joystick, bool isNumKeypad,
                              QWidget *parent = nullptr); // Accessed by pressing the "Quick Set" button
    ButtonEditDialog(QWidget *parent = 0);
    ~ButtonEditDialog();

    static ButtonEditDialog *getInstance();
    JoyButton *getLastJoyButton();
    void setUpLastJoyButton(JoyButton *);
    void refreshForLastBtn();
    void invokeMethodLastBtn(JoyButton *lastJoyBtn, ButtonEditDialogHelper *helper, const char *invokeString, int code,
                             int alias, int index, JoyButtonSlot::JoySlotInputAction mode, bool withClear, bool withTrue,
                             Qt::ConnectionType connTypeForAlias, Qt::ConnectionType connTypeForNothing,
                             Qt::ConnectionType connTypeForAll);

  protected:
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

  private:
    void setupVirtualKeyboardMouseTabWidget();

    ButtonEditDialogHelper helper;
    Ui::ButtonEditDialog *ui;
    ButtonEditDialogHelper &getHelperLocal();

    QElapsedTimer buttonEventInterval;
    InputDevice *joystick;
    QuickSetDialog *currentQuickDialog;
    bool ignoreRelease;
    bool withoutQuickSetDialog;
    bool m_isNumKeypad;
    JoyButton *lastJoyButton;

    static ButtonEditDialog *instance;

  signals:
    void advancedDialogOpened();
    void sendTempSlotToAdvanced(JoyButtonSlot *tempslot);
    void keyGrabbed(JoyButtonSlot *tempslot);
    void selectionCleared();
    void selectionFinished();
    void buttonDialogClosed();

  private slots:
    void nullifyDialogPointer();
    void refreshSlotSummaryLabel();
    void changeToggleSetting();
    void changeTurboSetting();
    void changeNumKeypadSetting();
    void openAdvancedDialog();
    void closedAdvancedDialog();
    void createTempSlot(int keycode, int alias);

    void checkTurboSetting(bool state);
    void setTurboButtonEnabled(bool state);
    void processSlotAssignment(JoyButtonSlot *tempslot);
    void clearButtonSlots();
    void sendSelectionFinished();
    void updateWindowTitleButtonName();
    void checkForKeyboardWidgetFocus(QWidget *old, QWidget *now);
};

#endif // BUTTONEDITDIALOGTWO_H
