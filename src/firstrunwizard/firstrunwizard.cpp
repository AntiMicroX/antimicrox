#include <QAbstractButton>

#include "firstrunwizard.h"

#include "firstrunwelcomepage.h"

#ifdef Q_OS_WIN
#include "associateprofilespage.h"
#include "winextras.h"
#endif

#include "mousesettingspage.h"

FirstRunWizard::FirstRunWizard(AntiMicroSettings *settings, QWidget *parent) :
    QWizard(parent)
{
    // Make sure instance is deleted when finished.
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("App Settings Wizard"));
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    setOption(QWizard::IndependentPages);

    this->settings = settings;

    setPage(WelcomePageID, new FirstRunWelcomePage(settings));

#if defined(Q_OS_WIN) && !defined(WIN_PORTABLE_PACKAGE)
    if (AssociateProfilesPage::shouldDisplay(settings))
    {
        setPage(AssociateProfilesPageID, new AssociateProfilesPage(settings));
    }

#endif

    if (MouseSettingsPage::shouldDisplay(settings))
    {
        setPage(MouseSettingsPageID, new MouseSettingsPage(settings));
    }

    button(QWizard::CancelButton)->setEnabled(false);

    connect(this, SIGNAL(finished(int)), this, SLOT(adjustSettings(int)));
}

void FirstRunWizard::adjustSettings(int status)
{
    Q_UNUSED(status);

#ifdef Q_OS_WIN
    if (hasVisitedPage(AssociateProfilesPageID))
    {
        bool shouldAssociateProfiles = field("associateProfiles").toBool();
        if (shouldAssociateProfiles)
        {
            settings->setValue("AssociateProfiles", "1");
        }
        else
        {
            settings->setValue("AssociateProfiles", "0");
        }

        if (!WinExtras::containsFileAssociationinRegistry() && shouldAssociateProfiles)
        {
            WinExtras::writeFileAssocationToRegistry();
        }
    }

#endif

    if (hasVisitedPage(MouseSettingsPageID))
    {
        settings->setValue("Mouse/Smoothing", field("mouseSmoothing").toBool() ? "1" : "0");
        settings->setValue("Mouse/HistorySize", field("historyBuffer").toInt());
        settings->setValue("Mouse/WeightModifier", field("weightModifier").toDouble());
        settings->setValue("Mouse/RefreshRate", field("mouseRefreshRate").toInt()+1);
#ifdef Q_OS_WIN
        settings->setValue("Mouse/DisableWinEnhancedPointer",
                           field("disableEnhancePrecision").toBool() ? "1" : "0");
#endif
    }

    // Save settings to file.
    settings->sync();
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
#if defined(Q_OS_WIN) && !defined(WIN_PORTABLE_PACKAGE)
    result = result || AssociateProfilesPage::shouldDisplay(settings);
#endif

    result = result || MouseSettingsPage::shouldDisplay(settings);

    return result;
}
