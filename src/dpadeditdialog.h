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

#ifndef DPADEDITDIALOG_H
#define DPADEDITDIALOG_H


#include "uihelpers/dpadeditdialoghelper.h"

#include <QDialog>

class JoyDPad;
class QWidget;

namespace Ui {
class DPadEditDialog;
}

class DPadEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DPadEditDialog(JoyDPad *dpad, QWidget *parent = nullptr);
    ~DPadEditDialog();

    JoyDPad* getDPad() const;
    DPadEditDialogHelper const& getHelper();

protected:
    void selectCurrentPreset();

private slots:
    void implementPresets(int index);
    void implementModes(int index);
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
    void updateWindowTitleDPadName();
    void updateDPadDelaySpinBox(int value);
    void updateDPadDelaySlider(double value);

private:
    Ui::DPadEditDialog *ui;
    DPadEditDialogHelper& getHelperLocal();

    JoyDPad *dpad;
    DPadEditDialogHelper helper;

};

#endif // DPADEDITDIALOG_H
