/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGlobal>
#include <QResource>
#include <QTextStream>

#ifdef USE_SDL_2
#include <SDL2/SDL_version.h>
#else
#include <SDL/SDL_version.h>
#endif

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

#include "eventhandlerfactory.h"

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

    BaseEventHandler *handler = 0;
    EventHandlerFactory *factory = EventHandlerFactory::getInstance();
    if (factory)
    {
        handler = factory->handler();
    }

    if (handler)
    {
        finalInfoText.append(tr("Using Event Handler: %1").arg(handler->getName()));
    }

    ui->infoTextBrowser->setText(finalInfoText.join("\n"));

    // Read Changelog text from resource and put text in text box.
    QResource changelogFile(":/Changelog");
    QFile temp(changelogFile.absoluteFilePath());
    temp.open(QIODevice::Text | QIODevice::ReadOnly);
    QTextStream changelogStream(&temp);
    QString changelogText = changelogStream.readAll();
    temp.close();
    ui->changelogPlainTextEdit->setPlainText(changelogText);
}

void AboutDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        retranslateUi();
    }

    QDialog::changeEvent(event);
}

void AboutDialog::retranslateUi()
{
    ui->retranslateUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);
}
