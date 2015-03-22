#include <QVBoxLayout>
#include <QLabel>

#include "associateprofilespage.h"
#include <winextras.h>

AssociateProfilesPage::AssociateProfilesPage(AntiMicroSettings *settings, QWidget *parent) :
    QWizardPage(parent)
{
    this->settings = settings;

    setTitle(tr("Associate Profiles"));
    setLayout(new QVBoxLayout);
    layout()->addWidget(new QLabel(
                            tr("Would you like to associate antimicro with .amgp files?")));

    associateCheckBox = new QCheckBox(tr("Associate"));
    layout()->addWidget(associateCheckBox);

    registerField("associateProfiles", associateCheckBox);
}

void AssociateProfilesPage::initializePage()
{
    bool shouldAssociateProfiles = settings->value("AssociateProfiles",
                                                   AntiMicroSettings::defaultAssociateProfiles).toBool();
    associateCheckBox->setChecked(shouldAssociateProfiles);
}

/**
 * @brief Determine whether the page should be initialized. Defaults to
 *   false.
 * @param AntiMicroSettings instance
 * @return If page instance should be created.
 */
bool AssociateProfilesPage::shouldDisplay(AntiMicroSettings *settings)
{
    bool result = false;
#if defined(Q_OS_WIN) && !defined(WIN_PORTABLE_PACKAGE)
    bool shouldAssociateProfiles = settings->value("AssociateProfiles",
                                                   AntiMicroSettings::defaultAssociateProfiles).toBool();

    if (!WinExtras::containsFileAssociationinRegistry() && shouldAssociateProfiles)
    {
        result = true;
    }
#else
    Q_UNUSED(settings);
#endif

    return result;
}
