#ifdef USE_SDL_2
#include <SDL2/SDL_version.h>
#else
#include <SDL/SDL_version.h>
#endif

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);

    QString sdlCompiledVersionNumber("%1.%2.%3");
    QString sdlLinkedVersionNumber("%1.%2.%3");

    SDL_version compiledver;
    SDL_version linkedver;
    SDL_VERSION(&compiledver);
#ifdef USE_SDL_2
    SDL_GetVersion(&linkedver);
#else
    linkedver = *(SDL_Linked_Version());
#endif

    sdlCompiledVersionNumber = sdlCompiledVersionNumber.arg(compiledver.major).arg(compiledver.minor).arg(compiledver.patch);
    QString sdlCompiledPlaceHolder = ui->sdlCompiledVersionLabel->text();
    sdlCompiledPlaceHolder = sdlCompiledPlaceHolder.arg(sdlCompiledVersionNumber);
    ui->sdlCompiledVersionLabel->setText(sdlCompiledPlaceHolder);

    sdlLinkedVersionNumber = sdlLinkedVersionNumber.arg(linkedver.major).arg(linkedver.minor).arg(linkedver.patch);
    QString sdlLinkedPlaceHolder = ui->sdlLinkedVersionLabel->text();
    sdlLinkedPlaceHolder = sdlLinkedPlaceHolder.arg(sdlLinkedVersionNumber);
    ui->sdlLinkedVersionLabel->setText(sdlLinkedPlaceHolder);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
