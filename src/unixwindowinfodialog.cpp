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
        ui->winClassCheckBox->setEnabled(false);
        ui->winClassCheckBox->setChecked(false);
    }
    else
    {
        ui->winClassCheckBox->setChecked(true);
        setRadioDefault = true;
    }

    winName = info->getWindowTitle(window);
    ui->winTitleLabel->setText(winName);
    if (winName.isEmpty())
    {
        ui->winTitleCheckBox->setEnabled(false);
        ui->winTitleCheckBox->setChecked(false);
    }
    else if (!setRadioDefault)
    {
        ui->winTitleCheckBox->setChecked(true);
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
                ui->winTitleCheckBox->setChecked(true);
                setRadioDefault = true;
            }
        }
        else
        {
            ui->winPathCheckBox->setEnabled(false);
            ui->winPathCheckBox->setChecked(false);
        }
    }
    else
    {
        ui->winPathCheckBox->setEnabled(false);
        ui->winPathCheckBox->setChecked(false);
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
    if (ui->winClassCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowClass;
    }

    if (ui->winTitleCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowName;
    }

    if (ui->winPathCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowPath;
    }
}

UnixWindowInfoDialog::DialogWindowOption UnixWindowInfoDialog::getSelectedOptions()
{
    return selectedMatch;
}

QString UnixWindowInfoDialog::getWindowClass()
{
    return winClass;
}

QString UnixWindowInfoDialog::getWindowName()
{
    return winName;
}

QString UnixWindowInfoDialog::getWindowPath()
{
    return winPath;
}

UnixWindowInfoDialog::~UnixWindowInfoDialog()
{
    delete ui;
}
