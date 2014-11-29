#include "qkeydisplaydialog.h"
#include "ui_qkeydisplaydialog.h"

#ifdef Q_OS_WIN
#include "winextras.h"
#endif

#ifdef Q_OS_UNIX
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif

#include "eventhandlerfactory.h"

    #if defined(WITH_UINPUT) && defined(WITH_X11)
#include "qtx11keymapper.h"

static QtX11KeyMapper x11KeyMapper;
    #endif

#include "event.h"

#endif

#include "antkeymapper.h"

QKeyDisplayDialog::QKeyDisplayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKeyDisplayDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setFocus();

#ifdef Q_OS_UNIX
    #if defined(WITH_UINPUT)
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
    ui->formLayout->removeWidget(ui->nativeTitleLabel);
    ui->formLayout->removeWidget(ui->nativeKeyLabel);
    ui->nativeTitleLabel->setVisible(false);
    ui->nativeKeyLabel->setVisible(false);
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
        #endif
    #endif

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
    ui->eventHandlerLabel->setText(handler->getName());

#else
    ui->formLayout->removeWidget(ui->eventHandlerTitleLabel);
    ui->formLayout->removeWidget(ui->eventHandlerLabel);
    ui->eventHandlerTitleLabel->setVisible(false);
    ui->eventHandlerLabel->setVisible(false);
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
#ifdef Q_OS_WIN
    unsigned int finalvirtual = WinExtras::correctVirtualKey(scancode, virtualkey);
#else

    unsigned int finalvirtual = 0;

    #ifdef WITH_X11
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        #endif
        // Obtain group 1 X11 keysym. Removes effects from modifiers.
        //finalvirtual = X11KeyCodeToX11KeySym(scancode);
        #ifdef WITH_UINPUT
        unsigned int tempalias = x11KeyMapper.returnQtKey(virtualkey);
        finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempalias);
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
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(finalvirtual, scancode), 0, 16);
#else
    QString tempValue = QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(finalvirtual), 0, 16);
#endif

    ui->antimicroKeyLabel->setText(tempValue);
}
