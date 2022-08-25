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

#include "common.h"
#include "eventhandlerfactory.h"

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_version.h>

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QResource>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(PadderCommon::programVersion);
    fillInfoTextBrowser();
}

AboutDialog::~AboutDialog() { delete ui; }

void AboutDialog::fillInfoTextBrowser()
{
    QStringList finalInfoText = QStringList();

    finalInfoText.append(tr("Program Version %1").arg(PadderCommon::programVersion));
#ifdef ANTIMICROX_PKG_VERSION
    finalInfoText.append(tr("Compiled from packaging: %1").arg(ANTIMICROX_PKG_VERSION));
#else
    finalInfoText.append(tr("Program Compiled on %1 at %2").arg(__DATE__).arg(__TIME__));
#endif

    finalInfoText.append(tr("Built Against SDL %1").arg(PadderCommon::sdlVersionCompiled));
    finalInfoText.append(tr("Running With SDL %1").arg(PadderCommon::sdlVersionUsed));

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

#ifdef Q_OS_LINUX
    QString detected_xdg_session = qgetenv("XDG_SESSION_TYPE");
    finalInfoText.append(QString("Compositor type: %1").arg(detected_xdg_session));
#endif

    finalInfoText.append(QString("Host OS: %1 Version: %2 Architecture: %3")
                             .arg(QSysInfo::productType(), QSysInfo::productVersion(), QSysInfo::currentCpuArchitecture()));

    ui->infoTextBrowser->setText(finalInfoText.join("\n"));

    // Read Changelog text from resource and put text in text box.
    QResource changelogFile(":/CHANGELOG.md");
    QFile temp(changelogFile.absoluteFilePath());
    temp.open(QIODevice::Text | QIODevice::ReadOnly);
    QTextStream changelogStream(&temp);
    QString changelogText = changelogStream.readAll();
    temp.close();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    ui->changelogText->setMarkdown(changelogText);
    ui->changelogText->setTextInteractionFlags(ui->changelogText->textInteractionFlags() | Qt::LinksAccessibleByMouse);
#else
    ui->changelogText->setPlainText(changelogText);
#endif
}

void AboutDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    QDialog::changeEvent(event);
}

void AboutDialog::retranslateUi()
{
    ui->retranslateUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);
}
