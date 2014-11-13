#include <QPushButton>

#include "capturedwindowinfodialog.h"
#include "ui_capturedwindowinfodialog.h"

#ifdef Q_OS_WIN
#include "wininfo.h"
#else
#include "x11info.h"
#endif

#ifdef Q_OS_WIN
CapturedWindowInfoDialog::CapturedWindowInfoDialog(QWidget *parent) :
#else
CapturedWindowInfoDialog::CapturedWindowInfoDialog(unsigned long window, QWidget *parent) :
#endif
    QDialog(parent),
    ui(new Ui::CapturedWindowInfoDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    selectedMatch = WindowNone;

#ifdef Q_OS_UNIX
    X11Info *info = X11Info::getInstance();
#endif

    bool setRadioDefault = false;
    fullWinPath = false;

#ifdef Q_OS_WIN
    ui->winClassCheckBox->setVisible(false);
    ui->winClassLabel->setVisible(false);
    ui->winClassHeadLabel->setVisible(false);
#else
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

    ui->winPathChoiceComboBox->setVisible(false);

#endif

#ifdef Q_OS_WIN
    winName = WinInfo::getCurrentWindowText();
#else
    winName = info->getWindowTitle(window);
#endif

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
#ifdef Q_OS_WIN
    winPath = WinInfo::getForegroundWindowExePath();
    ui->winPathLabel->setText(winPath);

    if (winPath.isEmpty())
    {
        ui->winPathCheckBox->setEnabled(false);
        ui->winPathCheckBox->setChecked(false);
    }
    else
    {
        ui->winPathCheckBox->setChecked(true);
        ui->winTitleCheckBox->setChecked(false);
        setRadioDefault = true;
    }

#elif defined(Q_OS_LINUX)
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
#endif

    if (winClass.isEmpty() && winName.isEmpty() &&
        winPath.isEmpty())
    {
        QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Ok);
        button->setEnabled(false);
    }

    connect(this, SIGNAL(accepted()), this, SLOT(populateOption()));
}

void CapturedWindowInfoDialog::populateOption()
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

        if (ui->winPathChoiceComboBox->currentIndex() == 0)
        {
            fullWinPath = true;
        }
        else
        {
            fullWinPath = false;
        }
    }
}

CapturedWindowInfoDialog::CapturedWindowOption CapturedWindowInfoDialog::getSelectedOptions()
{
    return selectedMatch;
}

QString CapturedWindowInfoDialog::getWindowClass()
{
    return winClass;
}

QString CapturedWindowInfoDialog::getWindowName()
{
    return winName;
}

QString CapturedWindowInfoDialog::getWindowPath()
{
    return winPath;
}

CapturedWindowInfoDialog::~CapturedWindowInfoDialog()
{
    delete ui;
}

bool CapturedWindowInfoDialog::useFullWindowPath()
{
    return fullWinPath;
}
