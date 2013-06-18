#include <QDebug>

#include "setaxisthrottledialog.h"
#include "ui_setaxisthrottledialog.h"

SetAxisThrottleDialog::SetAxisThrottleDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::SetAxisThrottleDialog)
{
    ui->setupUi(this);

    this->axis = axis;

    QString currentText = ui->label->text();
    currentText = currentText.arg(QString::number(axis->getRealJoyIndex()));
    ui->label->setText(currentText);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(propogateThrottleChange()));
    connect(this, SIGNAL(initiateSetAxisThrottleChange()), axis, SLOT(propogateThrottleChange()));
}

SetAxisThrottleDialog::~SetAxisThrottleDialog()
{
    delete ui;
}

void SetAxisThrottleDialog::propogateThrottleChange()
{
    emit initiateSetAxisThrottleChange();
}
