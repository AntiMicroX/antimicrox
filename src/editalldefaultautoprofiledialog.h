#ifndef EDITALLDEFAULTAUTOPROFILEDIALOG_H
#define EDITALLDEFAULTAUTOPROFILEDIALOG_H

#include <QDialog>
#include <QSettings>

#include "autoprofileinfo.h"


namespace Ui {
class EditAllDefaultAutoProfileDialog;
}

class EditAllDefaultAutoProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, QSettings *settings,
                                             QWidget *parent = 0);
    ~EditAllDefaultAutoProfileDialog();

protected:
    QString preferredProfileDir();

    AutoProfileInfo *info;
    QSettings *settings;

private:
    Ui::EditAllDefaultAutoProfileDialog *ui;

private slots:
    void openProfileBrowseDialog();
    void openApplicationBrowseDialog();
    void saveAutoProfileInformation();
};

#endif // EDITALLDEFAULTAUTOPROFILEDIALOG_H
