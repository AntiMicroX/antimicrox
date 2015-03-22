#include <QAbstractButton>

#include "firstrunwizard.h"

#ifdef Q_OS_WIN
#include "wizard/associateprofilespage.h"
#include "winextras.h"
#endif

#include "wizard/mousesettingspage.h"

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

    if (MouseSettingsPage::shouldDisplay(settings))
    {
        addPage(new MouseSettingsPage(settings));
    }

    button(QWizard::CancelButton)->setEnabled(false);

    connect(this, SIGNAL(finished(int)), this, SLOT(adjustSettings(int)));
}

void FirstRunWizard::adjustSettings(int status)
{
    Q_UNUSED(status);

#ifdef Q_OS_WIN
    if (field("associateProfiles").isValid())
    {
        bool shouldAssociateProfiles = field("associateProfiles").toBool();
        if (shouldAssociateProfiles)
        {
            settings->setValue("AssociateProfiles", 1);
        }
        else
        {
            settings->setValue("AssociateProfiles", 0);
        }

        if (!WinExtras::containsFileAssociationinRegistry() && shouldAssociateProfiles)
        {
            WinExtras::writeFileAssocationToRegistry();
        }
    }

#endif

    if (field("mouseSmoothing").isValid())
    {
        settings->setValue("Mouse/Smoothing", field("mouseSmoothing").toBool());
    }

    if (field("historyBuffer").isValid())
    {
        settings->setValue("Mouse/HistorySize", field("historyBuffer").toInt());
    }

    if (field("weightModifier").isValid())
    {
        settings->setValue("Mouse/WeightModifier", field("weightModifier").toDouble());
    }

    if (field("mouseRefreshRate").isValid())
    {
        settings->setValue("Mouse/RefreshRate", field("mouseRefreshRate").toInt()+1);
    }
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
#endif

    result = result || MouseSettingsPage::shouldDisplay(settings);

    return result;
}
