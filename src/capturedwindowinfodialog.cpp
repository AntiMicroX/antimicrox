#include <QPushButton>

#include "capturedwindowinfodialog.h"
#include "ui_capturedwindowinfodialog.h"

#ifdef Q_OS_WIN
#include "winextras.h"
#else
#include "x11extras.h"
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
    X11Extras *info = X11Extras::getInstance();
    ui->winPathChoiceComboBox->setVisible(false);
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
    winName = WinExtras::getCurrentWindowText();
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
    winPath = WinExtras::getForegroundWindowExePath();
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
        QString exepath = X11Extras::getInstance()->getApplicationLocation(pid);
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

CapturedWindowInfoDialog::~CapturedWindowInfoDialog()
{
    delete ui;
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

bool CapturedWindowInfoDialog::useFullWindowPath()
{
    return fullWinPath;
}
