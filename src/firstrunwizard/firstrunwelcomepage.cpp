#include <QLabel>
#include <QVBoxLayout>

#include "firstrunwelcomepage.h"

FirstRunWelcomePage::FirstRunWelcomePage(AntiMicroSettings *settings, QWidget *parent) :
    QWizardPage(parent)
{
    this->settings = settings;

    setTitle(tr("Welcome"));
    setLayout(new QVBoxLayout);

    QLabel *tempLabel = new QLabel(
                tr("Thank you for checking out antimicro. This "
                   "wizard can be used to customize some of the "
                   "program's behavior. More settings can be found "
                   "from the main interface under "
                   "Options > Settings."));
    tempLabel->setWordWrap(true);
    layout()->addWidget(tempLabel);
}
