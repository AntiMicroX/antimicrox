#include <QtGlobal>
#include <QLibraryInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QApplication>

#include "languageselectionpage.h"

LanguageSelectionPage::LanguageSelectionPage(AntiMicroSettings *settings, QTranslator *translator, QTranslator *appTranslator, QWidget *parent) :
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
            // Remove application specific translation strings
            qApp->removeTranslator(translator);

            // Remove old Qt translation strings
            qApp->removeTranslator(appTranslator);

            // Load new Qt translation strings
            appTranslator->load(QString("qt_").append(lang), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
            qApp->installTranslator(appTranslator);

            // Load application specific translation strings
        #if defined(Q_OS_UNIX)
            translator->load("antimicro_" + lang, QApplication::applicationDirPath().append("/../share/antimicro/translations"));
        #elif defined(Q_OS_WIN)
            translator->load("antimicro_" + lang, QApplication::applicationDirPath().append("\\share\\antimicro\\translations"));
        #endif
            qApp->installTranslator(translator);
        }
    }

    originalIndex = langListWidget->currentRow();
}
