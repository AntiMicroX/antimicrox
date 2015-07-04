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

#ifndef UNIXWINDOWINFODIALOG_H
#define UNIXWINDOWINFODIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class UnixWindowInfoDialog;
}

class UnixWindowInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnixWindowInfoDialog(unsigned long window, QWidget *parent = 0);
    ~UnixWindowInfoDialog();

    enum {
        WindowClass = (1 << 0),
        WindowName = (1 << 1),
        WindowPath = (1 << 2)
    };
    typedef unsigned int DialogWindowOption;

    QString getWindowClass();
    QString getWindowName();
    QString getWindowPath();
    DialogWindowOption getSelectedOptions();

private:
    Ui::UnixWindowInfoDialog *ui;

protected:
    DialogWindowOption selectedMatch;

    QString winClass;
    QString winName;
    QString winPath;

private slots:
    void populateOption();
};

#endif // UNIXWINDOWINFODIALOG_H
