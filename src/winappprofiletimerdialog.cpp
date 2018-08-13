#include "winappprofiletimerdialog.h"
#include "ui_winappprofiletimerdialog.h"

WinAppProfileTimerDialog::WinAppProfileTimerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WinAppProfileTimerDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(&appTimer, SIGNAL(timeout()), this, SLOT(accept()));
    connect(ui->capturePushButton, SIGNAL(clicked()), this, SLOT(startTimer()));
    connect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
}

WinAppProfileTimerDialog::~WinAppProfileTimerDialog()
{
    delete ui;
}

void WinAppProfileTimerDialog::startTimer()
{
    appTimer.start(ui->intervalSpinBox->value() * 1000);
    this->setEnabled(false);
}
