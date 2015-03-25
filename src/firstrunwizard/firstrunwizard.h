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
