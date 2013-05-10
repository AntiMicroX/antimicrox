#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->versionLabel->setText(PadderCommon::programVersion);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
