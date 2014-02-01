#include "qkeydisplaydialog.h"
#include "ui_qkeydisplaydialog.h"

#ifdef Q_OS_WIN
#include "wininfo.h"
#endif

#include "antkeymapper.h"

QKeyDisplayDialog::QKeyDisplayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKeyDisplayDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setFocus();
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
    unsigned int virtualkey = event->nativeVirtualKey();
#ifdef Q_OS_WIN
    unsigned int scancode = event->nativeScanCode();
    unsigned int finalvirtual = WinInfo::correctVirtualKey(scancode, virtualkey);
#else
    unsigned int finalvirtual = virtualkey;
#endif
    ui->nativeKeyLabel->setText(QString("0x%1").arg(finalvirtual, 0, 16));
    ui->qtKeyLabel->setText(QString("0x%1").arg(event->key(), 0, 16));

#ifdef Q_OS_WIN
    QString tempValue = QString("0x%1").arg(AntKeyMapper::returnQtKey(finalvirtual, scancode), 0, 16);
#else
    QString tempValue = QString("0x%1").arg(AntKeyMapper::returnQtKey(finalvirtual), 0, 16);
#endif

    ui->antimicroKeyLabel->setText(tempValue);
}
