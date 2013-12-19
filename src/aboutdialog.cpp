#ifdef USE_SDL_2
#include <SDL2/SDL_version.h>
#else
#include <SDL/SDL_version.h>
#endif

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);

    QString sdlVersionNumber("%1.%2.%3");
    SDL_version tempver;
    SDL_VERSION(&tempver);
    sdlVersionNumber = sdlVersionNumber.arg(tempver.major).arg(tempver.minor).arg(tempver.patch);
    QString sdlPlaceeholder = ui->sdlVersionLabel->text();
    sdlPlaceeholder = sdlPlaceeholder.arg(sdlVersionNumber);
    ui->sdlVersionLabel->setText(sdlPlaceeholder);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
