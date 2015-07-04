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

#ifndef ADDEDITAUTOPROFILEDIALOG_H
#define ADDEDITAUTOPROFILEDIALOG_H

#include <QDialog>

#include "autoprofileinfo.h"
#include "inputdevice.h"
#include "antimicrosettings.h"

namespace Ui {
class AddEditAutoProfileDialog;
}

class AddEditAutoProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings, QList<InputDevice*> *devices,
                                      QList<QString> &reservedGUIDS,
                                      bool edit=false, QWidget *parent = 0);
    ~AddEditAutoProfileDialog();

    AutoProfileInfo* getAutoProfile();
    QString getOriginalGUID();
    QString getOriginalExe();
    QString getOriginalWindowClass();
    QString getOriginalWindowName();

protected:
    virtual void accept();

    AutoProfileInfo *info;
    QList<InputDevice*> *devices;
    AntiMicroSettings *settings;
    bool editForm;
    bool defaultInfo;
    QList<QString> reservedGUIDs;
    QString originalGUID;
    QString originalExe;
    QString originalWindowClass;
    QString originalWindowName;

private:
    Ui::AddEditAutoProfileDialog *ui;

signals:
    void captureFinished();

private slots:
    void openProfileBrowseDialog();
    void openApplicationBrowseDialog();
    void saveAutoProfileInformation();
    void checkForReservedGUIDs(int index);
    void checkForDefaultStatus();
    void windowPropAssignment();

#ifdef Q_OS_WIN
    void openWinAppProfileDialog();
    void captureWindowsApplicationPath();
#else
    void showCaptureHelpWindow();
    void checkForGrabbedWindow();
#endif
};

#endif // ADDEDITAUTOPROFILEDIALOG_H
