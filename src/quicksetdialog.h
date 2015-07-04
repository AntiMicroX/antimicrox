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

#ifndef QUICKSETDIALOG_H
#define QUICKSETDIALOG_H

#include <QDialog>

#include "inputdevice.h"

namespace Ui {
class QuickSetDialog;
}

class QuickSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickSetDialog(InputDevice *joystick, QWidget *parent = 0);
    ~QuickSetDialog();

protected:
    InputDevice *joystick;
    QDialog *currentButtonDialog;

private:
    Ui::QuickSetDialog *ui;

signals:
    void buttonDialogClosed();

private slots:
    void showAxisButtonDialog();
    void showButtonDialog();
    void showStickButtonDialog();
    void showDPadButtonDialog();

    void nullifyDialogPointer();
    void restoreButtonStates();
};

#endif // QUICKSETDIALOG_H
