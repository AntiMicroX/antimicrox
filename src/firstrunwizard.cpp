#include "firstrunwizard.h"

#ifdef Q_OS_WIN
#include "wizard/associateprofilespage.h"
#include "winextras.h"
#endif

FirstRunWizard::FirstRunWizard(AntiMicroSettings *settings, QWidget *parent) :
    QWizard(parent)
{
    // Make sure instance is deleted when finished.
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("App Settings Wizard"));
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    this->settings = settings;

#ifdef Q_OS_WIN
    if (AssociateProfilesPage::shouldDisplay(settings))
    {
        addPage(new AssociateProfilesPage(settings));
    }

#endif

    connect(this, SIGNAL(finished(int)), this, SLOT(adjustSettings(int)));
}

void FirstRunWizard::adjustSettings(int status)
{
    Q_UNUSED(status);

#ifdef Q_OS_WIN
    bool shouldAssociateProfiles = field("associateProfiles").toBool();
    if (!WinExtras::containsFileAssociationinRegistry() && shouldAssociateProfiles)
    {
        WinExtras::writeFileAssocationToRegistry();
        settings->setValue("AssociateProfiles", 1);
    }
    else
    {
        settings->setValue("AssociateProfiles", 0);
    }
#endif
}

/**
 * @brief Determine whether the wizard should be initialized. Defaults to
 *   false.
 * @param AntiMicroSettings instance
 * @return If wizard instance should be created.
 */
bool FirstRunWizard::shouldDisplay(AntiMicroSettings *settings)
{
    bool result = false;
#if defined(Q_OS_WIN)
    result = AssociateProfilesPage::shouldDisplay(settings);
#else
    Q_UNUSED(settings);
#endif

    return result;
}
