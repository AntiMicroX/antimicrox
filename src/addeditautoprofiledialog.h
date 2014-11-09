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

#ifdef Q_OS_WIN
    void openWinAppProfileDialog();
    void captureWindowsApplicationPath();
#else
    void showCaptureHelpWindow();
    void checkForGrabbedWindow();
    void windowPropAssignment();
#endif
};

#endif // ADDEDITAUTOPROFILEDIALOG_H
