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
