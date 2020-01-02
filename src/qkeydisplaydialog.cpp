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

#include "qkeydisplaydialog.h"
#include "ui_qkeydisplaydialog.h"

#include "messagehandler.h"
#include "eventhandlerfactory.h"
#include "antkeymapper.h"

#include <QDebug>
#include <QKeySequence>
#include <QKeyEvent>
#include <QWidget>

#ifdef Q_OS_WIN
  #include "winextras.h"
#endif

#ifdef Q_OS_UNIX

#include <QApplication>

  #ifdef WITH_X11
    #include "x11extras.h"
  #endif
#endif



QKeyDisplayDialog::QKeyDisplayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKeyDisplayDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setFocus();

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
    ui->eventHandlerLabel->setText(handler->getName());
}

QKeyDisplayDialog::~QKeyDisplayDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    delete ui;
}

void QKeyDisplayDialog::keyPressEvent(QKeyEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    switch (event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        case Qt::Key_Return:
            break;

        default:
            QDialog::keyPressEvent(event);
    }
}

void QKeyDisplayDialog::keyReleaseEvent(QKeyEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int scancode = event->nativeScanCode();
    int virtualkey = event->nativeVirtualKey();

#ifdef Q_OS_WIN
    int finalvirtual = WinExtras::correctVirtualKey(scancode, virtualkey);
    int tempvirtual = finalvirtual;

  #ifdef WITH_VMULTI
    if (handler->getIdentifier() == "vmulti")
    {
        QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();

        if (nativeWinKeyMapper)
        {
            int tempQtKey = nativeWinKeyMapper->returnQtKey(finalvirtual);

            if (tempQtKey > 0)
                tempvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempQtKey);
        }
    }
  #endif
#elif defined(Q_OS_UNIX)

    int finalvirtual = 0;

    #ifdef WITH_X11

    if (QApplication::platformName() == QStringLiteral("xcb"))
{
        // Obtain group 1 X11 keysym. Removes effects from modifiers.
        finalvirtual = X11Extras::getInstance()->getGroup1KeySym(virtualkey);

        #ifdef WITH_UINPUT
        QtKeyMapperBase *nativeKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();

        if (nativeKeyMapper && (nativeKeyMapper->getIdentifier() == "xtest"))
        {
            int tempalias = nativeKeyMapper->returnQtKey(virtualkey);
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempalias);
        }
        #endif
    }
    else
    {
        finalvirtual = scancode;
    }

    #else

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
    }
    else
    {
        finalvirtual = scancode;
    }

    #endif
#endif

    ui->nativeKeyLabel->setText(QString("0x%1").arg(finalvirtual, 0, 16));
    ui->qtKeyLabel->setText(QString("0x%1").arg(event->key(), 0, 16));

#ifdef Q_OS_WIN
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(tempvirtual, scancode), 0, 16);
#elif defined(Q_OS_UNIX)
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(finalvirtual), 0, 16);
#endif

    ui->antimicroKeyLabel->setText(tempValue);
}
