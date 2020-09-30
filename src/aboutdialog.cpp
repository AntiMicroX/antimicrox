/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "messagehandler.h"
#include "common.h"
#include "eventhandlerfactory.h"

#include <SDL2/SDL_version.h>
#include <SDL2/SDL_gamecontroller.h>

#include <QtGlobal>
#include <QResource>
#include <QTextStream>
#include <QEvent>
#include <QFile>
#include <QStringList>
#include <QDebug>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->versionLabel->setText(PadderCommon::programVersion);
    fillInfoTextBrowser();
}

AboutDialog::~AboutDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void AboutDialog::fillInfoTextBrowser()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QStringList finalInfoText = QStringList();

    finalInfoText.append(tr("Program Version %1").arg(PadderCommon::programVersion));
    finalInfoText.append(tr("Program Compiled on %1 at %2").arg(__DATE__).arg(__TIME__));

    QString sdlCompiledVersionNumber("%1.%2.%3");
    QString sdlLinkedVersionNumber("%1.%2.%3");

    SDL_version compiledver;
    SDL_version linkedver;
    SDL_VERSION(&compiledver);
    SDL_GetVersion(&linkedver);

    sdlCompiledVersionNumber = sdlCompiledVersionNumber.arg(compiledver.major).arg(compiledver.minor).arg(compiledver.patch);
    finalInfoText.append(tr("Built Against SDL %1").arg(sdlCompiledVersionNumber));

    sdlLinkedVersionNumber = sdlLinkedVersionNumber.arg(linkedver.major).arg(linkedver.minor).arg(linkedver.patch);
    finalInfoText.append(tr("Running With SDL %1").arg(sdlLinkedVersionNumber));

    finalInfoText.append(tr("Using Qt %1").arg(qVersion()));

    BaseEventHandler *handler = nullptr;
    EventHandlerFactory *factory = EventHandlerFactory::getInstance();

    if (factory != nullptr)
    {
        handler = factory->handler();
    }

    if (handler != nullptr)
    {
        finalInfoText.append(tr("Using Event Handler: %1").arg(handler->getName()));
    }

    ui->infoTextBrowser->setText(finalInfoText.join("\n"));

    // Read Changelog text from resource and put text in text box.
    QResource changelogFile(":/CHANGELOG.md");
    QFile temp(changelogFile.absoluteFilePath());
    temp.open(QIODevice::Text | QIODevice::ReadOnly);
    QTextStream changelogStream(&temp);
    QString changelogText = changelogStream.readAll();
    temp.close();
    ui->changelogPlainTextEdit->setPlainText(changelogText);
}

void AboutDialog::changeEvent(QEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (event->type() == QEvent::LanguageChange) retranslateUi();

    QDialog::changeEvent(event);
}

void AboutDialog::retranslateUi()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->retranslateUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);
}
