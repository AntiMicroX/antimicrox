#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include <QWizard>

#include "antimicrosettings.h"

class FirstRunWizard : public QWizard
{
    Q_OBJECT
public:
    explicit FirstRunWizard(AntiMicroSettings *settings, QWidget *parent = 0);

    static bool shouldDisplay(AntiMicroSettings *settings);

protected:
    AntiMicroSettings *settings;

signals:

private slots:
    void adjustSettings(int status);
};

#endif // FIRSTRUNWIZARD_H
