#include "qkeydisplaydialog.h"
#include "ui_qkeydisplaydialog.h"

#ifdef Q_OS_WIN
#include "qtwinkeymapper.h"
#else
#include "qtx11keymapper.h"
#endif

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
    ui->nativeKeyLabel->setText(QString("0x%1").arg(event->nativeVirtualKey(), 0, 16));
    ui->qtKeyLabel->setText(QString("0x%1").arg(event->key(), 0, 16));
#ifdef Q_OS_WIN
    QString tempValue = QString("0x%1").arg(QtWinKeyMapper::returnQtKey(event->nativeVirtualKey()), 0, 16);
    ui->antimicroKeyLabel->setText(tempValue);
#else
    QString tempValue = QString("0x%1").arg(QtX11KeyMapper::returnQtKey(event->nativeVirtualKey()), 0, 16);
    ui->antimicroKeyLabel->setText(tempValue);
#endif
}
