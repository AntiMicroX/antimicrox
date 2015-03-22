#ifndef FIRSTRUNWELCOMEPAGE_H
#define FIRSTRUNWELCOMEPAGE_H

#include <QWizardPage>

#include <antimicrosettings.h>

class FirstRunWelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit FirstRunWelcomePage(AntiMicroSettings *settings, QWidget *parent = 0);

    static bool shouldDisplay(AntiMicroSettings *settings);

protected:
    AntiMicroSettings *settings;

signals:

public slots:

};

#endif // FIRSTRUNWELCOMEPAGE_H
