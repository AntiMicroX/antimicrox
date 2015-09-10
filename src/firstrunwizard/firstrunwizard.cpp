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

#include <QAbstractButton>

#include "firstrunwizard.h"

#include "firstrunwelcomepage.h"

#ifdef Q_OS_WIN
#include "associateprofilespage.h"
#include "winextras.h"
#endif

#include "mousesettingspage.h"
#include "languageselectionpage.h"

FirstRunWizard::FirstRunWizard(AntiMicroSettings *settings, QTranslator *translator,
                               QTranslator *appTranslator, QWidget *parent) :
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
    this->translator = translator;
    this->appTranslator = appTranslator;

    setPage(WelcomePageID, new FirstRunWelcomePage(settings));
    setPage(LanguageSelectionPageID, new LanguageSelectionPage(settings, translator, appTranslator));

#if defined(Q_OS_WIN) && !defined(WIN_PORTABLE_PACKAGE)
    if (AssociateProfilesPage::shouldDisplay(settings))
    {
        setPage(AssociateProfilesPageID, new AssociateProfilesPage(settings));
    }

#endif

    setPage(MouseSettingsPageID, new MouseSettingsPage(settings));

    button(QWizard::CancelButton)->setEnabled(false);

    connect(this, SIGNAL(finished(int)), this, SLOT(adjustSettings(int)));
}

void FirstRunWizard::adjustSettings(int status)
{
    Q_UNUSED(status);

    settings->getLock()->lock();

    if (hasVisitedPage(LanguageSelectionPageID))
    {
        QString tempLang = LanguageSelectionPage::languageForIndex(field("selectedLanguage").toInt());
        if (tempLang != QLocale::system().name())
        {
            settings->setValue("Language", tempLang);
            // Call for re-translation. Will be skipped if already called.
            LanguageSelectionPage *langPage = static_cast<LanguageSelectionPage*>(page(LanguageSelectionPageID));
            langPage->retranslateUi();
        }
    }

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

    // Flag Wizard as having been used
    settings->setValue("WizardUsed", "1");

    // Save settings to file.
    settings->sync();

    settings->getLock()->unlock();
}

int FirstRunWizard::nextId() const
{
    // Passed the language selection page. Alter language setting if needed.
    if (currentId() == MouseSettingsPageID)
    {
        LanguageSelectionPage *langPage = static_cast<LanguageSelectionPage*>(page(LanguageSelectionPageID));
        langPage->retranslateUi();
    }

    return QWizard::nextId();
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

    settings->getLock()->lock();
    // Only show wizard if no saved settings exist.
    if (settings->allKeys().size() == 0)
    {
        result = true;
    }
    settings->getLock()->unlock();

    return result;
}
