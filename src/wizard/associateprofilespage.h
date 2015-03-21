#ifndef ASSOCIATEPROFILESPAGE_H
#define ASSOCIATEPROFILESPAGE_H

#include <QWizardPage>
#include <QCheckBox>

#include <antimicrosettings.h>

class AssociateProfilesPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit AssociateProfilesPage(AntiMicroSettings *settings, QWidget *parent = 0);

    virtual void initializePage();

    static bool shouldDisplay(AntiMicroSettings *settings);

protected:
    AntiMicroSettings *settings;

    QCheckBox *associateCheckBox;

signals:

public slots:

};

#endif // ASSOCIATEPROFILESPAGE_H
