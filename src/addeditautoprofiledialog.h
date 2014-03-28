#ifndef ADDEDITAUTOPROFILEDIALOG_H
#define ADDEDITAUTOPROFILEDIALOG_H

#include <QDialog>
#include <QSettings>

#include "autoprofileinfo.h"
#include "inputdevice.h"

namespace Ui {
class AddEditAutoProfileDialog;
}

class AddEditAutoProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditAutoProfileDialog(AutoProfileInfo *info, QSettings *settings, QList<InputDevice*> *devices,
                                      QList<QString> &reservedGUIDS,
                                      bool edit=false, QWidget *parent = 0);
    ~AddEditAutoProfileDialog();

    AutoProfileInfo* getAutoProfile();
    QString getOriginalGUID();
    QString getOriginalExe();

protected:
    QString preferredProfileDir();
    virtual void accept();
    bool validateForm();

    AutoProfileInfo *info;
    QList<InputDevice*> *devices;
    QSettings *settings;
    bool editForm;
    bool defaultInfo;
    QList<QString> reservedGUIDs;
    QString originalGUID;
    QString originalExe;

private:
    Ui::AddEditAutoProfileDialog *ui;

private slots:
    void openProfileBrowseDialog();
    void openApplicationBrowseDialog();
    void saveAutoProfileInformation();
    void checkForReservedGUIDs(int index);
    void checkForDefaultStatus(QString text);
#if defined(Q_OS_UNIX)
    void selectWindowWithCursor();
#elif defined(Q_OS_WIN)
    void openWinAppProfileDialog();
    void captureWindowsApplicationPath();
#endif
};

#endif // ADDEDITAUTOPROFILEDIALOG_H
