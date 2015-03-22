#ifndef MOUSESMOOTHINGPAGE_H
#define MOUSESMOOTHINGPAGE_H

#include <QWizardPage>

#include <antimicrosettings.h>

class MouseSettingsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit MouseSettingsPage(AntiMicroSettings *settings, QWidget *parent = 0);

    static bool shouldDisplay(AntiMicroSettings *settings);

protected:
    AntiMicroSettings *settings;

signals:

public slots:

};

#endif // MOUSESMOOTHINGPAGE_H
