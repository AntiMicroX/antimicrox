#ifndef LANGUAGESELECTIONPAGE_H
#define LANGUAGESELECTIONPAGE_H

#include <QWizardPage>
#include <QTranslator>
#include <QListWidget>

#include <antimicrosettings.h>

class LanguageSelectionPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LanguageSelectionPage(AntiMicroSettings *settings, QTranslator *translator,
                                   QTranslator *appTranslator, QWidget *parent = 0);

    static QString languageForIndex(int currentIndex);

protected:
    AntiMicroSettings *settings;
    QTranslator *translator;
    QTranslator *appTranslator;
    int originalIndex;
    QListWidget *langListWidget;

signals:

public slots:
    void retranslateUi();
};

#endif // LANGUAGESELECTIONPAGE_H
