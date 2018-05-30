#include "calibration.h"
#include "ui_calibration.h"

#include "joycontrolstick.h"
#include "joytabwidget.h"
#include "inputdevice.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"

#include <SDL2/SDL_joystick.h>

#include <QTabWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QLayoutItem>
#include <QPointer>


Calibration::Calibration(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibration),
    helper(joysticks->value(0)->getSetJoystick(0)->getJoyStick(0))
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(trUtf8("Calibration"));

    this->joysticks = joysticks;
    QPointer<JoyControlStick> controlstick = joysticks->value(0)->getSetJoystick(0)->getJoyStick(0);
    this->stick = controlstick.data();
    controlstick.data()->getModifierButton()->establishPropertyUpdatedConnections();
    helper.moveToThread(controlstick.data()->thread());

    setProgressBars(0, 0, 0);

    ui->stickStatusBoxWidget->setStick(controlstick.data());
    ui->stickStatusBoxWidget->update();

    if (controlstick.isNull())
        controlstick.clear();

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
    if (enoughProb()) {

        calculateDeadZone(x_es_val);
        calculateDeadZone(y_es_val);

    } else {


    }
}


bool Calibration::enoughProb()
{
    bool enough = true;
    if (x_es_val.count(QString("+")) < 3) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move X axis to the right at least three times!")); }
    else if (x_es_val.count(QString("-")) < 3) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move X axis to the left at least three times!")); }
    else if (y_es_val.count(QString("+")) < 3) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move Y axis up at least three times!")); }
    else if (y_es_val.count(QString("-")) < 3) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move Y axis down at least three times!")); }

    return enough;
}


int Calibration::calculateDeadZone(QHash<QString,int> ax_values)
{
    // temporarily
    return 0;
}


void Calibration::checkX(int value)
{
    if (value > 0) {
        if (x_es_val.count(QString("+")) <= 5) x_es_val.insertMulti(QString("+"), value);
    } else {
        if (x_es_val.count(QString("-")) <= 5) x_es_val.insertMulti(QString("-"), value);
    }

    axisBarX->setValue(value);
    update();
}


void Calibration::checkY(int value)
{
    if (value > 0) {
        if (y_es_val.count(QString("+")) <= 5) y_es_val.insertMulti(QString("+"), value);
    } else {
        if (y_es_val.count(QString("-")) <= 5) y_es_val.insertMulti(QString("-"), value);
    }

    axisBarY->setValue(value);
    update();
}


void Calibration::setController(QString controllerName)
{
    QMapIterator<SDL_JoystickID, InputDevice*> iterTemp(*joysticks);

    while (iterTemp.hasNext())
    {
        iterTemp.next();

        QPointer<InputDevice> joystick = iterTemp.value();

        if (controllerName == joystick.data()->getSDLName())
        {
            updateAxesBox();
            createAxesConnection();
            break;
        }

        if (joystick.isNull()) joystick.clear();
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

    QPointer<JoyControlStick> controlstick = joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getJoyStick(ui->axesBox->currentIndex());
    this->stick = controlstick.data();
    controlstick.data()->getModifierButton()->establishPropertyUpdatedConnections();
    helper.moveToThread(controlstick.data()->thread());
    ui->stickStatusBoxWidget->setStick(controlstick.data());
    ui->stickStatusBoxWidget->update();
    setProgressBars(controlstick.data());

    if (controlstick.isNull()) controlstick.clear();
}


void Calibration::setProgressBars(JoyControlStick* controlstick)
{

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

            connect(joyAxisX, &JoyAxis::moved, this, &Calibration::checkX);
            connect(joyAxisY, &JoyAxis::moved, this, &Calibration::checkY);

        }

        update();
}


void Calibration::setProgressBars(int inputDevNr, int setJoyNr, int stickNr)
{

        JoyControlStick* controlstick = joysticks->value(inputDevNr)->getSetJoystick(setJoyNr)->getJoyStick(stickNr);
        helper.moveToThread(controlstick->thread());

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

            connect(joyAxisX, &JoyAxis::moved, this, &Calibration::checkX);
            connect(joyAxisY, &JoyAxis::moved, this, &Calibration::checkY);

        }

        update();
}
