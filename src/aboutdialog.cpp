#include <QtGlobal>

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
    fillInfoTextBrowser();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::fillInfoTextBrowser()
{
    QStringList finalInfoText;

    finalInfoText.append(tr("Program Version %1").arg(PadderCommon::programVersion));
    finalInfoText.append(tr("Program Compiled on %1 at %2").arg(__DATE__).arg(__TIME__));

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
    finalInfoText.append(tr("Built Against SDL %1").arg(sdlCompiledVersionNumber));

    sdlLinkedVersionNumber = sdlLinkedVersionNumber.arg(linkedver.major).arg(linkedver.minor).arg(linkedver.patch);
    finalInfoText.append(tr("Running With SDL %1").arg(sdlLinkedVersionNumber));

    finalInfoText.append(tr("Using Qt %1").arg(qVersion()));

    ui->infoTextBrowser->setText(finalInfoText.join("\n"));
}
