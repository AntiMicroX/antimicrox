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

#ifndef JOYCONTROLSTICKEDITDIALOG_H
#define JOYCONTROLSTICKEDITDIALOG_H

#include <QDialog>

#include "joycontrolstick.h"
#include "uihelpers/joycontrolstickeditdialoghelper.h"

namespace Ui {
class JoyControlStickEditDialog;
}

class JoyControlStickEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent = 0);
    ~JoyControlStickEditDialog();
    
protected:
    void selectCurrentPreset();

    JoyControlStick *stick;
    JoyControlStickEditDialogHelper helper;

private:
    Ui::JoyControlStickEditDialog *ui;

private slots:
    void implementPresets(int index);
    void implementModes(int index);

    void refreshStickStats(int x, int y);
    void updateMouseMode(int index);

    void checkMaxZone(int value);
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
    void updateWindowTitleStickName();
    void changeCircleAdjust(int value);
    void updateStickDelaySpinBox(int value);
    void updateStickDelaySlider(double value);
    void openModifierEditDialog();
    void changeModifierSummary();
};

#endif // JOYCONTROLSTICKEDITDIALOG_H
