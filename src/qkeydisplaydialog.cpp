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

//#include <QDebug>
#include <QKeySequence>

#include "qkeydisplaydialog.h"
#include "ui_qkeydisplaydialog.h"

#include "eventhandlerfactory.h"
#include "antkeymapper.h"

#ifdef Q_OS_WIN
  #include "winextras.h"
#endif

#ifdef Q_OS_UNIX
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <QApplication>
  #endif

  #ifdef WITH_X11
    #include "x11extras.h"
  #endif
#endif


QKeyDisplayDialog::QKeyDisplayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKeyDisplayDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setFocus();

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
    ui->eventHandlerLabel->setText(handler->getName());

#ifdef Q_OS_UNIX
    #if defined(WITH_UINPUT)
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    /*ui->formLayout->removeWidget(ui->nativeTitleLabel);
    ui->formLayout->removeWidget(ui->nativeKeyLabel);
    ui->nativeTitleLabel->setVisible(false);
    ui->nativeKeyLabel->setVisible(false);
    */
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #endif

#else
    /*ui->formLayout->removeWidget(ui->eventHandlerTitleLabel);
    ui->formLayout->removeWidget(ui->eventHandlerLabel);
    ui->eventHandlerTitleLabel->setVisible(false);
    ui->eventHandlerLabel->setVisible(false);
    */
#endif

}

QKeyDisplayDialog::~QKeyDisplayDialog()
{
    delete ui;
}

void QKeyDisplayDialog::keyPressEvent(QKeyEvent *event)
{
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
    unsigned int scancode = event->nativeScanCode();
    unsigned int virtualkey = event->nativeVirtualKey();

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
#ifdef Q_OS_WIN
    unsigned int finalvirtual = WinExtras::correctVirtualKey(scancode, virtualkey);
    unsigned int tempvirtual = finalvirtual;

  #ifdef WITH_VMULTI
    if (handler->getIdentifier() == "vmulti")
    {
        QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
        if (nativeWinKeyMapper)
        {
            unsigned int tempQtKey = nativeWinKeyMapper->returnQtKey(finalvirtual);
            if (tempQtKey > 0)
            {
                tempvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempQtKey);
            }
        }
    }
  #endif
#else

    unsigned int finalvirtual = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
        // Obtain group 1 X11 keysym. Removes effects from modifiers.
        finalvirtual = X11Extras::getInstance()->getGroup1KeySym(virtualkey);

        #ifdef WITH_UINPUT
        unsigned int tempalias = 0;
        QtKeyMapperBase *nativeKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
        if (nativeKeyMapper && nativeKeyMapper->getIdentifier() == "xtest")
        {
            tempalias = nativeKeyMapper->returnQtKey(virtualkey);
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempalias);
        }
        #endif

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    else
    {
        finalvirtual = scancode;
    }
        #endif

    #else
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(event->key());
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    else
    {
        finalvirtual = scancode;
    }
        #endif
    #endif
#endif

    ui->nativeKeyLabel->setText(QString("0x%1").arg(finalvirtual, 0, 16));
    ui->qtKeyLabel->setText(QString("0x%1").arg(event->key(), 0, 16));

#ifdef Q_OS_WIN
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(tempvirtual, scancode), 0, 16);
#else
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(finalvirtual), 0, 16);
#endif

    ui->antimicroKeyLabel->setText(tempValue);
}
