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

#include <QtGlobal>
#include <QLibraryInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QApplication>

#include "languageselectionpage.h"
#include "common.h"

LanguageSelectionPage::LanguageSelectionPage(AntiMicroSettings *settings,
                                             QTranslator *translator,
                                             QTranslator *appTranslator,
                                             QWidget *parent) :
    QWizardPage(parent)
{
    this->settings = settings;
    this->translator = translator;
    this->appTranslator = appTranslator;
    this->originalIndex = 0;

    setTitle(tr("Language"));
    setSubTitle(tr("Select a language"));
    QVBoxLayout *tempLayout = new QVBoxLayout;
    setLayout(tempLayout);

    QLabel *tempLabel = new QLabel(
                tr("antimicro has been translated into many different "
                   "languages by contributors. By default, the program will "
                   "choose an appropriate translation based on your system's "
                   "locale setting. However, you can make antimicro load a "
                   "different translation depending on the language that you "
                   "choose from the list below."));
    tempLabel->setWordWrap(true);
    tempLayout->addWidget(tempLabel);

    tempLayout->addSpacerItem(new QSpacerItem(10, 10));

    langListWidget = new QListWidget;
    langListWidget->addItem(tr("Default"));
    langListWidget->addItem(tr("Brazilian Portuguese"));
    langListWidget->addItem(tr("English"));
    langListWidget->addItem(tr("French"));
    langListWidget->addItem(tr("German"));
    langListWidget->addItem(tr("Russian"));
    langListWidget->addItem(tr("Serbian"));
    langListWidget->addItem(tr("Simplified Chinese"));
    langListWidget->addItem(tr("Ukrainian"));

    tempLayout->addWidget(langListWidget);

    registerField("selectedLanguage", langListWidget);
}

QString LanguageSelectionPage::languageForIndex(int currentIndex)
{
    QString temp = QLocale::system().name();
    if (currentIndex == 1)
    {
        temp = "br";
    }
    else if (currentIndex == 2)
    {
        temp = "en";
    }
    else if (currentIndex == 3)
    {
        temp = "fr";
    }
    else if (currentIndex == 4)
    {
        temp = "de";
    }
    else if (currentIndex == 5)
    {
        temp = "ru";
    }
    else if (currentIndex == 6)
    {
        temp = "sr";
    }
    else if (currentIndex == 7)
    {
        temp = "zh_CN";
    }
    else if (currentIndex == 8)
    {
        temp = "uk";
    }

    return temp;
}

void LanguageSelectionPage::retranslateUi()
{
    if (langListWidget->currentRow() != originalIndex)
    {
        QString lang = languageForIndex(field("selectedLanguage").toInt());
        if (!lang.isEmpty())
        {
            PadderCommon::reloadTranslations(translator, appTranslator, lang);
        }
    }

    originalIndex = langListWidget->currentRow();
}
