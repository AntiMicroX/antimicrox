#include "calibration.h"
#include "ui_calibration.h"

#include "joycontrolstick.h"
#include "joytabwidget.h"
#include "inputdevice.h"

#include <SDL2/SDL_joystick.h>

#include <QTabWidget>
#include <QProgressBar>


Calibration::Calibration(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibration),
    helper(stick)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    // TODO - iterate over all sticks of selected/current controller

    this->joysticks = joysticks;
    this->stick = joysticks->value(0)->getSetJoystick(0)->getJoyStick(0);

    /*
     * // for getSetJoystick(int) is InputDevice::NUMBER_JOYSETS
    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis != nullptr)
        {
            QHBoxLayout *hbox = new QHBoxLayout();

            QLabel *axisLabel = new QLabel();
            axisLabel->setText(trUtf8("Axis %1").arg(axis->getRealJoyIndex()));
            QProgressBar *axisBar = new QProgressBar();
            axisBar->setMinimum(JoyAxis::AXISMIN);
            axisBar->setMaximum(JoyAxis::AXISMAX);
            axisBar->setFormat("%v");
            axisBar->setValue(axis->getCurrentRawValue());
            hbox->addWidget(axisLabel);
            hbox->addWidget(axisBar);
            hbox->addSpacing(10);
            axesBox->addLayout(hbox);

            connect(axis, &JoyAxis::moved, axisBar, &QProgressBar::setValue);
        }
    }
    */

    SetJoystick *currentset = joysticks->value(0)->getSetJoystick(0);

    for (int i = 0; i < joysticks->value(0)->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

      /*QHBoxLayout *hbox = new QHBoxLayout();

        QLabel *axisLabel = new QLabel();
        axisLabel->setText(trUtf8("%1").arg(axis->getRealJoyIndex()));
        QProgressBar *axisBar = new QProgressBar();
        axisBar->setMinimum(JoyAxis::AXISMIN);
        axisBar->setMaximum(JoyAxis::AXISMAX);
        axisBar->setFormat("%v");
        axisBar->setValue(axis->getCurrentRawValue());*/

        // or maybe dynamically
        ui->labelX->setText(trUtf8("%1").arg(axis->getRealJoyIndex()));

        ui->movedXProgress->setMinimum(JoyAxis::AXISMIN);
        ui->movedXProgress->setMaximum(JoyAxis::AXISMAX);
        ui->movedXProgress->setFormat("%v");
        ui->movedXProgress->setValue(axis->getCurrentRawValue());
    }

    for (int j = 0; j < joysticks->value(0)->getNumberSticks(); j++)
    {
        JoyControlStick *controlstick = currentset->getJoyStick(j);
        ui->axesBox->addItem(controlstick->getStickName());
    }

    helper.moveToThread(stick->thread());

    ui->stickStatusBoxWidget->setStick(stick);

    QMapIterator<SDL_JoystickID, InputDevice*> iterTemp(*joysticks);

    while (iterTemp.hasNext())
    {
        iterTemp.next();

        InputDevice *joystick = iterTemp.value();
        QString joytabName = joystick->getSDLName();
        ui->controllersBox->addItem(joytabName);
    }

    connect(ui->saveBtn, &QPushButton::clicked, this, &Calibration::saveSettings);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &Calibration::close);
    connect(ui->movedXProgress, &QProgressBar::valueChanged, this, &Calibration::checkX);
    connect(ui->movedYProgress, &QProgressBar::valueChanged, this, &Calibration::checkY);
    connect(ui->controllersBox, &QComboBox::currentTextChanged, this, &Calibration::setController);
    connect(ui->axesBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Calibration::createAxesConnection);
    connect(ui->jstestgtkCheckbox, &QCheckBox::stateChanged, this, &Calibration::loadSetFromJstest);

    update();
}


Calibration::~Calibration()
{
    delete ui;
}


void Calibration::saveSettings()
{

}


void Calibration::checkX()
{

}


void Calibration::checkY()
{

}


void Calibration::setController(QString controllerName)
{
    QMapIterator<SDL_JoystickID, InputDevice*> iterTemp(*joysticks);

    while (iterTemp.hasNext())
    {
        iterTemp.next();

        InputDevice *joystick = iterTemp.value();
        // TODO - iterate over all sticks
        JoyControlStick* joyStick = iterTemp.value()->getSetJoystick(0)->getJoyStick(0);
        QString joytabName = joystick->getSDLName();

        if (controllerName == joytabName) {
            stick = joyStick;
            ui->stickStatusBoxWidget->setStick(stick);
            update();
            break;
        }
    }
}


void Calibration::loadSetFromJstest()
{

}


void Calibration::createAxesConnection()
{

}

