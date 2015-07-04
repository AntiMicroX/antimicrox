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

#ifndef EDITALLDEFAULTAUTOPROFILEDIALOG_H
#define EDITALLDEFAULTAUTOPROFILEDIALOG_H

#include <QDialog>

#include "autoprofileinfo.h"
#include "antimicrosettings.h"

namespace Ui {
class EditAllDefaultAutoProfileDialog;
}

class EditAllDefaultAutoProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
                                             QWidget *parent = 0);
    ~EditAllDefaultAutoProfileDialog();

    AutoProfileInfo* getAutoProfile();

protected:
    virtual void accept();

    AutoProfileInfo *info;
    AntiMicroSettings *settings;

private:
    Ui::EditAllDefaultAutoProfileDialog *ui;

private slots:
    void openProfileBrowseDialog();
    void saveAutoProfileInformation();
};

#endif // EDITALLDEFAULTAUTOPROFILEDIALOG_H
