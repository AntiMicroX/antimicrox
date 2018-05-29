#include "calibration.h"
#include "ui_calibration.h"

#include "joycontrolstick.h"
#include "joytabwidget.h"
#include "inputdevice.h"

#include <SDL2/SDL_joystick.h>

#include <QTabWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QLayoutItem>


Calibration::Calibration(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibration),
    helper(stick)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    // TODO - THREADS

    this->joysticks = joysticks;
    this->stick = joysticks->value(0)->getSetJoystick(0)->getJoyStick(0);


    // for getSetJoystick(int) is InputDevice::NUMBER_JOYSETS
    // for (int i = 0; i < joystick->getNumberAxes(); i++)

   /* SetJoystick *currentset = joysticks->value(0)->getSetJoystick(0);

    for (int i = 0; i < joysticks->value(0)->getNumberAxes(); i++)
    {
        JoyAxis *axis = currentset->getJoyAxis(i);

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
            ui->progressBarsLayout->addLayout(hbox);

            connect(axis, &JoyAxis::moved, axisBar, &QProgressBar::setValue);
        }
    }*/

    setProgressBars(0, 0, 0);

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

    for (int i = 0; i < joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getNumberSticks(); i++)
    {
       ui->axesBox->addItem(joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getJoyStick(i)->getPartialName());
    }

    connect(ui->saveBtn, &QPushButton::clicked, this, &Calibration::saveSettings);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &Calibration::close);
  //  connect(ui->movedXProgress, &QProgressBar::valueChanged, this, &Calibration::checkX);
  //  connect(ui->movedYProgress, &QProgressBar::valueChanged, this, &Calibration::checkY);
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

        if (controllerName == joystick->getSDLName()) {
            stick = joyStick;
            updateAxesBox();
            createAxesConnection();
            break;
        }
    }
}


void Calibration::updateAxesBox()
{
    ui->axesBox->clear();

    for (int i = 0; i < joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getNumberSticks(); i++)
    {
       ui->axesBox->addItem(joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getJoyStick(i)->getPartialName());
    }

    update();
}


void Calibration::loadSetFromJstest()
{

}


void Calibration::createAxesConnection()
{
    qDeleteAll(ui->axesWidget->findChildren<QWidget*>());

    if ((joyAxisX != nullptr) && (axisBarX != nullptr)) {
        disconnect(joyAxisX, &JoyAxis::moved, axisBarX, nullptr);
       // delete joyAxisX;
       // delete axisBarX;
        joyAxisX = nullptr;
        axisBarX = nullptr;
    }

    if ((joyAxisY != nullptr) && (axisBarY != nullptr)) {
        disconnect(joyAxisY, &JoyAxis::moved, axisBarY, nullptr);
       // delete joyAxisY;
       // delete axisBarY;
        joyAxisY = nullptr;
        axisBarY = nullptr;
    }

    setProgressBars(ui->controllersBox->currentIndex(), 0, ui->axesBox->currentIndex());
    helper.moveToThread(stick->thread());
    this->stick = joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getJoyStick(ui->axesBox->currentIndex());
    ui->stickStatusBoxWidget->setStick(stick);
}


void Calibration::setProgressBars(int inputDevNr, int setJoyNr, int stickNr)
{

        JoyControlStick* controlstick = joysticks->value(inputDevNr)->getSetJoystick(setJoyNr)->getJoyStick(stickNr);

        joyAxisX = controlstick->getAxisX();
        joyAxisY = controlstick->getAxisY();

        if ((joyAxisX != nullptr) && (joyAxisY != nullptr))
        {
            QHBoxLayout *hbox = new QHBoxLayout();
            QHBoxLayout *hbox2 = new QHBoxLayout();

            QLabel *axisLabel = new QLabel();
            QLabel *axisLabel2 = new QLabel();
            axisLabel->setText(trUtf8("Axis %1").arg(joyAxisX->getRealJoyIndex()));
            axisLabel2->setText(trUtf8("Axis %1").arg(joyAxisY->getRealJoyIndex()));
            axisBarX = new QProgressBar();
            axisBarY = new QProgressBar();
            axisBarX->setMinimum(JoyAxis::AXISMIN);
            axisBarX->setMaximum(JoyAxis::AXISMAX);
            axisBarX->setFormat("%v");
            axisBarX->setValue(joyAxisX->getCurrentRawValue());
            axisBarY->setMinimum(JoyAxis::AXISMIN);
            axisBarY->setMaximum(JoyAxis::AXISMAX);
            axisBarY->setFormat("%v");
            axisBarY->setValue(joyAxisY->getCurrentRawValue());
            hbox->addWidget(axisLabel);
            hbox->addWidget(axisBarX);
            hbox->addSpacing(10);
            hbox2->addWidget(axisLabel2);
            hbox2->addWidget(axisBarY);
            hbox2->addSpacing(10);
            ui->progressBarsLayout->addLayout(hbox);
            ui->progressBarsLayout->addLayout(hbox2);

            connect(joyAxisX, &JoyAxis::moved, axisBarX, &QProgressBar::setValue);
            connect(joyAxisY, &JoyAxis::moved, axisBarY, &QProgressBar::setValue);
        }

        update();
}

