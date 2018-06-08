#include "calibration.h"
#include "ui_calibration.h"

#include "joycontrolstick.h"
#include "joytabwidget.h"
#include "inputdevice.h"
#include "messagehandler.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"

#include <SDL2/SDL_joystick.h>

#include <QTabWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QLayoutItem>
#include <QPointer>
#include <QDebug>


Calibration::Calibration(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibration),
    helper(joysticks->value(0)->getSetJoystick(0)->getJoyStick(0))
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(trUtf8("Calibration"));

    sumX = 0;
    sumY = 0;
    center_calibrated_x = -1;
    center_calibrated_y = -1;
    max_axis_val_x = -1;
    min_axis_val_x = -1;
    max_axis_val_y = -1;
    min_axis_val_y = -1;
    deadzone_calibrated_x = -1;
    deadzone_calibrated_y = -1;
    calibrated = false;

    this->joysticks = joysticks;
    QPointer<JoyControlStick> controlstick = joysticks->value(0)->getSetJoystick(0)->getJoyStick(0);
    this->stick = controlstick.data();
    calibrated = this->stick->wasCalibrated();
    ui->Informations->setText(stick->getCalibrationSummary());

    controlstick.data()->getModifierButton()->establishPropertyUpdatedConnections();
    helper.moveToThread(controlstick.data()->thread());

    setProgressBars(0, 0, 0);
    ui->stickStatusBoxWidget->setFocus();

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
    connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startCalibration);
    connect(ui->resetBtn, &QPushButton::clicked, this, &Calibration::resetSettings);


    update();
}


Calibration::~Calibration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void Calibration::resetSettings()
{
    QMessageBox msgBox;
    msgBox.setText(trUtf8("Do you really want to reset settings of current axes?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    switch(msgBox.exec())
    {
        case QMessageBox::Ok:
        sumX = 0;
        sumY = 0;
        center_calibrated_x = -1;
        center_calibrated_y = -1;
        max_axis_val_x = -1;
        min_axis_val_x = -1;
        max_axis_val_y = -1;
        min_axis_val_y = -1;
        deadzone_calibrated_x = -1;
        deadzone_calibrated_y = -1;
        calibrated = false;
        x_es_val.clear();
        y_es_val.clear();

        joyAxisX->setAxisCenterCal(center_calibrated_x);
        joyAxisY->setAxisCenterCal(center_calibrated_y);
        joyAxisX->setDeadZone(JoyAxis::AXISDEADZONE);
        joyAxisY->setDeadZone(JoyAxis::AXISDEADZONE);
        joyAxisX->setAxisMinCal(min_axis_val_x);
        joyAxisY->setAxisMinCal(min_axis_val_y);
        joyAxisX->setAxisMaxCal(max_axis_val_x);
        joyAxisY->setAxisMaxCal(max_axis_val_y);
        joyAxisX->setMaxZoneValue(JoyAxis::AXISMAXZONE);
        joyAxisY->setMaxZoneValue(JoyAxis::AXISMAXZONE);
        stick->setDeadZone(JoyAxis::AXISDEADZONE);
        stick->setMaxZone(JoyAxis::AXISMAXZONE);
        calibrated = false;
        stick->setCalibrationFlag(false);
        stick->setCalibrationSummary(QString());

        update();
        ui->stickStatusBoxWidget->update();
        break;

        case QMessageBox::Cancel:
        break;

        default:
        break;

    }
}

void Calibration::setQuadraticZoneCalibrated(int &max_axis_val_x, int &min_axis_val_x, int &max_axis_val_y, int &min_axis_val_y) {

    if(max_axis_val_x > abs(min_axis_val_x)) {
        max_axis_val_x = abs(min_axis_val_x);
    } else {
        min_axis_val_x = -(max_axis_val_x);
    }

    if(max_axis_val_y > abs(min_axis_val_y)) {
        max_axis_val_y = abs(min_axis_val_y);
    } else {
        min_axis_val_y = -(max_axis_val_y);
    }
}


int Calibration::calibratedDeadZone(int center, int deadzone)
{
    return (center + deadzone);
}


void Calibration::startCalibration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool confirmed = true;

    if (stick->wasCalibrated()) {

        QMessageBox msgBox;
        msgBox.setText(trUtf8("Calibration was saved for the preset. Do you really want to reset settings?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        switch(msgBox.exec())
        {
            case QMessageBox::Ok:
                confirmed = true;
            break;

            case QMessageBox::Cancel:
                confirmed = false;
            break;

            default:
                confirmed = true;
            break;

        }
    }


    if ((joyAxisX != nullptr) && (joyAxisY != nullptr) && confirmed) {

        stick->setCalibrationFlag(false);
        calibrated = false;

            ui->steps->setText(trUtf8("place the joystick in the center position"));
            update();

            this->setWindowTitle(trUtf8("calibrating center"));
            ui->startButton->setText(trUtf8("Start second step"));
            update();

                for (int i = 0; i < x_es_val.count(); i++) {
                  sumX += x_es_val.values().at(i);
                }

                for (int i = 0; i < y_es_val.count(); i++) {
                  sumY += y_es_val.values().at(i);
                }

                if ((sumX != 0) && (sumY != 0)) {
                    center_calibrated_x = sumX / x_es_val.count();
                    center_calibrated_y = sumY / y_es_val.count();
                } else {
                    center_calibrated_x = 0;
                    center_calibrated_y = 0;
                }

               // deadzone_calibrated_x = calibratedDeadZone(center_calibrated_x, joyAxisX->getDeadZone());
               // deadzone_calibrated_y = calibratedDeadZone(center_calibrated_y, joyAxisY->getDeadZone());

                QString text = QString();
                text.append(trUtf8("\n\nCenter X: %1").arg(center_calibrated_x));
                text.append(trUtf8("\nCenter Y: %1").arg(center_calibrated_y));
                ui->Informations->setText(text);
                this->text = text;

                x_es_val.clear();
                y_es_val.clear();
                sumX = 0;
                sumY = 0;
                update();

                disconnect(ui->startButton, &QPushButton::clicked, this, nullptr);
                connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startSecondStep);

    }
}


void Calibration::startSecondStep()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joyAxisX != nullptr) && (joyAxisY != nullptr)) {

            ui->steps->setText(trUtf8("\nplace the joystick in the top-left corner many times"));
            update();

            this->setWindowTitle(trUtf8("calibrating position"));
            update();

            qDebug() << "X_ES_VAL: " << x_es_val.count(QString("-"));
            qDebug() << "Y_ES_VAL: " << y_es_val.count(QString("-"));


            if (enoughProb(x_es_val.count(QString("-")), y_es_val.count(QString("-")))) {

                int min_x = 0;
                int min_y = 0;

                for (int i = 0; i < x_es_val.count(QString("-")); i++) {
                  //  if ((abs(x_es_val.values(QString("-")).at(i)) > deadzone_calibrated_x))
                    if (i > 0) {
                        if (x_es_val.values(QString("-")).at(i) < x_es_val.values(QString("-")).at(i-1))  {
                            min_x = x_es_val.values(QString("-")).at(i);
                        }
                    } else {
                        min_x = x_es_val.values(QString("-")).at(i);
                    }
                }

                for (int i = 0; i < y_es_val.count(QString("-")); i++) {
                  //  if ((abs(y_es_val.values(QString("-")).at(i)) > deadzone_calibrated_y))
                    if (i > 0) {
                        if (y_es_val.values(QString("-")).at(i) < y_es_val.values(QString("-")).at(i-1))  {
                            min_y = y_es_val.values(QString("-")).at(i);
                        }
                    } else {
                        min_y = y_es_val.values(QString("-")).at(i);
                    }
                }

                min_axis_val_x = min_x;
                min_axis_val_y = min_y;

                QString text = ui->Informations->text();
                text.append(trUtf8("\n\nX: %1").arg(min_axis_val_x));
                text.append(trUtf8("\nY: %1").arg(min_axis_val_y));
                ui->Informations->setText(text);
                this->text = text;
                update();

                x_es_val.clear();
                y_es_val.clear();
                sumX = 0;
                sumY = 0;

                disconnect(ui->startButton, &QPushButton::clicked, this, nullptr);
                connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startLastStep);
        }
    }
}


void Calibration::startLastStep()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joyAxisX != nullptr) && (joyAxisY != nullptr)) {

            ui->steps->setText(trUtf8("\nplace the joystick in the bottom-right corner"));
            update();

            this->setWindowTitle(trUtf8("calibrating position"));
            ui->startButton->setText(trUtf8("Start final step"));
            update();

            if (enoughProb(x_es_val.count(QString("+")), y_es_val.count(QString("+")))) {

                int max_x = 0;
                int max_y = 0;

                for (int i = 0; i < x_es_val.count(QString("+")); i++) {
                  //  if ((x_es_val.values(QString("+")).at(i) > deadzone_calibrated_x))
                        if (i > 0) {
                            if (x_es_val.values(QString("+")).at(i) > x_es_val.values(QString("+")).at(i-1))  {
                                max_x = x_es_val.values(QString("+")).at(i);
                            }
                        } else {
                            max_x = x_es_val.values(QString("+")).at(i);
                        }
                }

                for (int i = 0; i < y_es_val.count(QString("+")); i++) {
                 //   if ((y_es_val.values(QString("+")).at(i) > deadzone_calibrated_y))
                    if (i > 0) {
                        if (y_es_val.values(QString("+")).at(i) > y_es_val.values(QString("+")).at(i-1))  {
                            max_y = y_es_val.values(QString("+")).at(i);
                        }
                    } else {
                        max_y = y_es_val.values(QString("+")).at(i);
                    }
                }

                max_axis_val_x = max_x;
                max_axis_val_y = max_y;

                QString text2 = ui->Informations->text();
                text2.append(trUtf8("\n\nX: %1").arg(max_axis_val_x));
                text2.append(trUtf8("\nY: %1").arg(max_axis_val_y));
                ui->Informations->setText(text2);
                this->text = text2;
                update();

                setQuadraticZoneCalibrated(max_axis_val_x, min_axis_val_x, max_axis_val_y, min_axis_val_y);

                deadzone_calibrated_x = (max_axis_val_y + max_axis_val_x)/4;
                deadzone_calibrated_y = (max_axis_val_y + max_axis_val_x)/4;

                QString text3 = ui->Informations->text();
                text3.append(trUtf8("\n\nrange X: %1 - %2").arg(min_axis_val_x).arg(max_axis_val_x));
                text3.append(trUtf8("\nrange Y: %1 - %2").arg(min_axis_val_y).arg(max_axis_val_y));
                text3.append(trUtf8("\n\ndeadzone X: %1").arg(deadzone_calibrated_x));
                text3.append(trUtf8("\ndeadzone Y: %1").arg(deadzone_calibrated_y));
                ui->Informations->setText(text3);
                this->text = text3;

                ui->steps->setText(trUtf8("\n---calibration done---\n"));
                ui->startButton->setText(trUtf8("Start calibration"));
                this->setWindowTitle(trUtf8("Calibration"));
                update();

                disconnect(ui->startButton, &QPushButton::clicked, this, nullptr);
                connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startCalibration);

            }
      }
}


void Calibration::saveSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


   if ((joyAxisX != nullptr) && (joyAxisY != nullptr)) {

      joyAxisX->setAxisCenterCal(center_calibrated_x);
      joyAxisY->setAxisCenterCal(center_calibrated_y);
      joyAxisX->setDeadZone(deadzone_calibrated_x);
      joyAxisY->setDeadZone(deadzone_calibrated_y);
      stick->setDeadZone(deadzone_calibrated_x);
      joyAxisX->setAxisMinCal(min_axis_val_x);
      joyAxisY->setAxisMinCal(min_axis_val_y);
      joyAxisX->setAxisMaxCal(max_axis_val_x);
      joyAxisY->setAxisMaxCal(max_axis_val_y);
      joyAxisX->setMaxZoneValue(max_axis_val_x);
      joyAxisY->setMaxZoneValue(max_axis_val_y);
      stick->setMaxZone(max_axis_val_x);
      calibrated = true;
      stick->setCalibrationFlag(true);
      stick->setCalibrationSummary(this->text);

      ui->stickStatusBoxWidget->update();
      update();
      QMessageBox::information(this, trUtf8("Save"), trUtf8("Calibration values have been saved"));
   }
}


const QString Calibration::getSetfromGtkJstest()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return QString();
}


bool Calibration::enoughProb(int x_count, int y_count)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool enough = true;

    if (x_count < 5) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move X axis to the right at least five times! Keep moving!")); }
    else if (y_count < 5) { enough = false; QMessageBox::information(this, trUtf8("Dead zone calibration"), trUtf8("You must move X axis to the left at least five times! Keep moving!")); }

    return enough;
}


int Calibration::calculateRawVal(QHash<QString,int> ax_values, JoyAxis* joyAxis)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int min_int = chooseMinMax(QString("-"), ax_values.values(QString("-")));
    int max_int = chooseMinMax(QString("+"), ax_values.values(QString("+")));
    int deadzone_value = 0;

    qDebug() << "Int Min value of " << joyAxis->getAxisName() << " is " << min_int;
    qDebug() << "Int Max value of " << joyAxis->getAxisName() << " is " << max_int;

    if (abs(min_int) == max_int) {

        qDebug() << "Min and max values are equal";

        deadzone_value = max_int;

    } else {

        if (max_int > abs(min_int)) {

            qDebug() << "Max value is greater than min value";

            deadzone_value = min_int;

        } else {

            qDebug() << "Min value is greater than max value";

            deadzone_value = max_int;

        }
    }

    joyAxis->setCurrentRawValue(deadzone_value);
    int axis1Value = joyAxis->getCurrentRawValue();

    qDebug() << "current raw value for joyAxis is " << axis1Value;

    return axis1Value;
}


int Calibration::chooseMinMax(QString min_max_sign, QList<int> ax_values)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int min_max = 0;

    foreach(int val, ax_values) {

        if (min_max_sign == QString("+")) {
            if (min_max < val) min_max = val;

        } else {
            if (min_max > val) min_max = val;
        }
    }

    return min_max;
}


void Calibration::checkX(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value > 0) {
        if (x_es_val.count(QString("+")) <= 100) x_es_val.insert(QString("+"), value);
    } else if (value < 0) {
        if (x_es_val.count(QString("-")) <= 100) x_es_val.insert(QString("-"), value);
    }

    axisBarX->setValue(value);
    update();
}


void Calibration::checkY(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value > 0) {
        if (y_es_val.count(QString("+")) <= 100) y_es_val.insert(QString("+"), value);
    } else if (value < 0) {
        if (y_es_val.count(QString("-")) <= 100) y_es_val.insert(QString("-"), value);
    }

    axisBarY->setValue(value);
    update();
}


void Calibration::setController(QString controllerName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->axesBox->clear();

    for (int i = 0; i < joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getNumberSticks(); i++)
    {
       ui->axesBox->addItem(joysticks->value(ui->controllersBox->currentIndex())->getSetJoystick(0)->getJoyStick(i)->getPartialName());
    }

    update();
}


void Calibration::loadSetFromJstest()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

}


bool Calibration::ifGtkJstestRunToday()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return true;
}


void Calibration::createAxesConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        joyAxisX = controlstick->getAxisX();
        joyAxisY = controlstick->getAxisY();

        calibrated = controlstick->wasCalibrated();
        ui->Informations->setText(controlstick->getCalibrationSummary());

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        JoyControlStick* controlstick = joysticks->value(inputDevNr)->getSetJoystick(setJoyNr)->getJoyStick(stickNr);
        helper.moveToThread(controlstick->thread());

        joyAxisX = controlstick->getAxisX();
        joyAxisY = controlstick->getAxisY();

        calibrated = controlstick->wasCalibrated();
        ui->Informations->setText(controlstick->getCalibrationSummary());

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
