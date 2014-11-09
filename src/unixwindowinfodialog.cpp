#include <QPushButton>

#include "unixwindowinfodialog.h"
#include "ui_unixwindowinfodialog.h"

#include "x11info.h"

UnixWindowInfoDialog::UnixWindowInfoDialog(unsigned long window, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnixWindowInfoDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    X11Info *info = X11Info::getInstance();
    bool setRadioDefault = false;

    winClass = info->getWindowClass(window);
    ui->winClassLabel->setText(winClass);
    if (winClass.isEmpty())
    {
        ui->winClassRadioButton->setEnabled(false);
        ui->winClassRadioButton->setChecked(false);
    }
    else
    {
        ui->winClassRadioButton->setChecked(true);
        setRadioDefault = true;
    }

    winName = info->getWindowTitle(window);
    ui->winTitleLabel->setText(winName);
    if (winName.isEmpty())
    {
        ui->winTitleRadioButton->setEnabled(false);
        ui->winTitleRadioButton->setChecked(false);
    }
    else if (!setRadioDefault)
    {
        ui->winTitleRadioButton->setChecked(true);
        setRadioDefault = true;
    }

    ui->winPathLabel->clear();
    int pid = info->getApplicationPid(window);
    if (pid > 0)
    {
        QString exepath = X11Info::getInstance()->getApplicationLocation(pid);
        if (!exepath.isEmpty())
        {
            ui->winPathLabel->setText(exepath);
            winPath = exepath;
            if (!setRadioDefault)
            {
                ui->winTitleRadioButton->setChecked(true);
                setRadioDefault = true;
            }
        }
        else
        {
            ui->winPathRadioButton->setEnabled(false);
            ui->winPathRadioButton->setChecked(false);
        }
    }
    else
    {
        ui->winPathRadioButton->setEnabled(false);
        ui->winPathRadioButton->setChecked(false);
    }

    if (winClass.isEmpty() && winName.isEmpty() &&
        winPath.isEmpty())
    {
        QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Ok);
        button->setEnabled(false);
    }

    connect(this, SIGNAL(accepted()), this, SLOT(populateOption()));
}

void UnixWindowInfoDialog::populateOption()
{
    if (ui->winClassRadioButton->isChecked())
    {
        selectedMatch = WindowClass;
        selectedValue = winClass;
    }
    else if (ui->winTitleRadioButton->isChecked())
    {
        selectedMatch = WindowName;
        selectedValue = winName;
    }
    else if (ui->winPathRadioButton->isChecked())
    {
        selectedMatch = WindowPath;
        selectedValue = winPath;
    }
}

QString UnixWindowInfoDialog::getPropertyValue()
{
    return selectedValue;
}

UnixWindowInfoDialog::DialogWindowOption UnixWindowInfoDialog::getSelectedOption()
{
    return selectedMatch;
}

UnixWindowInfoDialog::~UnixWindowInfoDialog()
{
    delete ui;
}
