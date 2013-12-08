#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QDebug>

#include "joystickstatuswindow.h"
#include "ui_joystickstatuswindow.h"
#include "joybuttonstatusbox.h"


JoystickStatusWindow::JoystickStatusWindow(Joystick *joystick, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JoystickStatusWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;

    setWindowTitle(tr("%1 (#%2) Properties").arg(joystick->getSDLName())
                   .arg(joystick->getRealJoyNumber()));

    ui->joystickNameLabel->setText(joystick->getSDLName());
    ui->joystickNumberLabel->setText(QString::number(joystick->getRealJoyNumber()));
    ui->joystickAxesLabel->setText(QString::number(joystick->getNumberAxes()));
    ui->joystickButtonsLabel->setText(QString::number(joystick->getNumberButtons()));
    ui->joystickHatsLabel->setText(QString::number(joystick->getNumberHats()));

    joystick->getActiveSetJoystick()->setIgnoreEventState(true);
    joystick->getActiveSetJoystick()->release();
    joystick->resetButtonDownCount();

    QVBoxLayout *axesBox = new QVBoxLayout();
    axesBox->setSpacing(4);
    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis)
        {
            QHBoxLayout *hbox = new QHBoxLayout();

            QLabel *axisLabel = new QLabel();
            axisLabel->setText(tr("Axis %1").arg(axis->getRealJoyIndex()));
            QProgressBar *axisBar = new QProgressBar();
            axisBar->setMinimum(JoyAxis::AXISMIN);
            axisBar->setMaximum(JoyAxis::AXISMAX);
            axisBar->setFormat("%v");
            axisBar->setValue(axis->getCurrentRawValue());
            hbox->addWidget(axisLabel);
            hbox->addWidget(axisBar);
            hbox->addSpacing(10);
            axesBox->addLayout(hbox);

            connect(axis, SIGNAL(moved(int)), axisBar, SLOT(setValue(int)));
        }
    }

    ui->axesScrollArea->setLayout(axesBox);

    QGridLayout *buttonsGrid = new QGridLayout();
    buttonsGrid->setHorizontalSpacing(4);
    buttonsGrid->setVerticalSpacing(10);

    int currentRow = 0;
    int currentColumn = 0;
    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button)
        {
            JoyButtonStatusBox *statusbox = new JoyButtonStatusBox(button);
            statusbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            statusbox->setMinimumSize(30, 30);
            buttonsGrid->addWidget(statusbox, currentRow, currentColumn);
            currentColumn++;
            if (currentColumn >= 6)
            {
                currentRow++;
                currentColumn = 0;
            }
        }
    }

    ui->buttonsScrollArea->setLayout(buttonsGrid);

    QVBoxLayout *hatsBox = new QVBoxLayout();
    hatsBox->setSpacing(4);
    for (int i=0; i < joystick->getNumberHats(); i++)
    {
        JoyDPad *dpad = joystick->getActiveSetJoystick()->getJoyDPad(i);
        if (dpad)
        {
            QHBoxLayout *hbox = new QHBoxLayout();

            QLabel *dpadLabel = new QLabel();
            dpadLabel->setText(tr("Hat %1").arg(dpad->getRealJoyNumber()));
            QProgressBar *dpadBar = new QProgressBar();
            dpadBar->setMinimum(JoyDPadButton::DpadCentered);
            dpadBar->setMaximum(JoyDPadButton::DpadLeftDown);
            dpadBar->setFormat("%v");
            dpadBar->setValue(dpad->getCurrentDirection());
            hbox->addWidget(dpadLabel);
            hbox->addWidget(dpadBar);
            hbox->addSpacing(10);
            hatsBox->addLayout(hbox);

            connect(dpad, SIGNAL(active(int)), dpadBar, SLOT(setValue(int)));
            connect(dpad, SIGNAL(released(int)), dpadBar, SLOT(setValue(int)));
        }
    }

    hatsBox->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Fixed));

    if (ui->hatsGroupBox->layout())
    {
        delete ui->hatsGroupBox->layout();
    }

    ui->hatsGroupBox->setLayout(hatsBox);

    connect(this, SIGNAL(finished(int)), this, SLOT(restoreButtonStates()));
}

JoystickStatusWindow::~JoystickStatusWindow()
{
    delete ui;
}

void JoystickStatusWindow::restoreButtonStates()
{
    joystick->getActiveSetJoystick()->setIgnoreEventState(false);
    joystick->getActiveSetJoystick()->release();
}
