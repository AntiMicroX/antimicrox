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

#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QWizard>
#include <QTranslator>

#include "antimicrosettings.h"

class FirstRunWizard : public QWizard
{
    Q_OBJECT
public:
    enum {
        WelcomePageID,
        LanguageSelectionPageID,
        AssociateProfilesPageID,
        MouseSettingsPageID
    };

    explicit FirstRunWizard(AntiMicroSettings *settings, QTranslator *translator,
                            QTranslator *appTranslator, QWidget *parent = 0);

    virtual int nextId() const;

    static bool shouldDisplay(AntiMicroSettings *settings);

protected:
    AntiMicroSettings *settings;
    QTranslator *translator;
    QTranslator *appTranslator;

signals:

private slots:
    void adjustSettings(int status);
};

#endif // FIRSTRUNWIZARD_H
