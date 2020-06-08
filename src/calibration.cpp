/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "calibration.h"
#include "ui_calibration.h"

#include "globalvariables.h"
#include "joycontrolstick.h"
#include "joytabwidget.h"
#include "inputdevice.h"
#include "messagehandler.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"

#include <SDL2/SDL_joystick.h>

#include <QtConcurrent>
#include <QFuture>
#include <QTabWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QLayoutItem>
#include <QPointer>
#include <QDebug>



Calibration::Calibration(InputDevice* joystick, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Calibration),
    currentJoystick(joystick),
    helper(currentJoystick->getActiveSetJoystick()->getJoyStick(0))
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setAttribute( Qt::WA_DeleteOnClose, true );
    setWindowTitle(tr("Calibration"));

    sumX = 0;
    sumY = 0;

    center_calibrated_x = 0;
    center_calibrated_y = 0;

    deadzone_calibrated_x = 0;
    deadzone_calibrated_y = 0;

    max_axis_val_x = 0;
    min_axis_val_x = 0;

    max_axis_val_y = 0;
    min_axis_val_y = 0;

    calibrated = false;

    QPointer<JoyControlStick> controlstick = currentJoystick->getActiveSetJoystick()->getJoyStick(0);
    this->stick = controlstick.data();
    calibrated = this->stick->wasCalibrated();
    ui->Informations->setText(stick->getCalibrationSummary());

    ui->resetBtn->setEnabled(calibrated);
    ui->saveBtn->setEnabled(false);

    controlstick.data()->getModifierButton()->establishPropertyUpdatedConnections();
    helper.moveToThread(controlstick.data()->thread());

    setProgressBars(0, 0);
    ui->stickStatusBoxWidget->setFocus();
    ui->stickStatusBoxWidget->setStick(controlstick.data());
    ui->stickStatusBoxWidget->update();

    if (controlstick.isNull())
        controlstick.clear();

    QList<JoyControlStick*> sticksList = currentJoystick->getActiveSetJoystick()->getSticks().values();
    QListIterator<JoyControlStick*> currStick(sticksList);

    while (currStick.hasNext())
    {
        ui->axesBox->addItem(currStick.next()->getPartialName());
    }

    connect(currentJoystick, &InputDevice::destroyed, this, &Calibration::close);
    connect(ui->saveBtn, &QPushButton::clicked, this, &Calibration::saveSettings);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &Calibration::close);
    connect(ui->axesBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Calibration::createAxesConnection);
    connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startCalibration);
    connect(ui->resetBtn, &QPushButton::clicked, [this](bool clicked)
    {
       resetSettings(false, clicked);
    });

    update();
}


Calibration::~Calibration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

/**
 * @brief Resets memory of all variables to default, updates window and shows message
 * @return Nothing
 */
void Calibration::resetSettings(bool silentReset, bool)
{
    if (!silentReset) {

        QMessageBox msgBox;
        msgBox.setText(tr("Do you really want to reset settings of current axis?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        switch(msgBox.exec())
        {
            case QMessageBox::Yes:
                restoreCalValues();
                ui->steps->clear();
            break;

            case QMessageBox::No:
            break;

            default:
            break;
        }
    }
    else
    {
        restoreCalValues();
        ui->steps->clear();
    }
}

void Calibration::restoreCalValues()
{
    sumX = 0;
    sumY = 0;

    center_calibrated_x = 0;
    center_calibrated_y = 0;
    deadzone_calibrated_x = 0;
    deadzone_calibrated_y = 0;

    max_axis_val_x = 0;
    min_axis_val_x = 0;
    max_axis_val_y = 0;
    min_axis_val_y = 0;

    x_es_val.clear();
    y_es_val.clear();

    joyAxisX->setAxisCenterCal(center_calibrated_x);
    joyAxisY->setAxisCenterCal(center_calibrated_y);

    joyAxisX->setAxisMinCal(GlobalVariables::JoyAxis::AXISMIN);
    joyAxisY->setAxisMinCal(GlobalVariables::JoyAxis::AXISMIN);
    joyAxisX->setAxisMaxCal(GlobalVariables::JoyAxis::AXISMAX);
    joyAxisY->setAxisMaxCal(GlobalVariables::JoyAxis::AXISMAX);

    joyAxisX->setDeadZone(GlobalVariables::JoyAxis::AXISDEADZONE);
    joyAxisY->setDeadZone(GlobalVariables::JoyAxis::AXISDEADZONE);

    joyAxisX->setMaxZoneValue(GlobalVariables::JoyAxis::AXISMAXZONE);
    joyAxisY->setMaxZoneValue(GlobalVariables::JoyAxis::AXISMAXZONE);

    stick->setDeadZone(GlobalVariables::JoyAxis::AXISDEADZONE);
    stick->setMaxZone(GlobalVariables::JoyAxis::AXISMAXZONE);

    stick->setCalibrationFlag(false);
    stick->setCalibrationSummary(QString());
    ui->Informations->clear();

    calibrated = false;

    ui->saveBtn->setEnabled(false);
    ui->resetBtn->setEnabled(false);
    ui->stickStatusBoxWidget->update();

    update();
}

/**
 * @brief Creates quadratic field in a case, when max value is not equal to negative min value. It always chooses less value
 * @param max value for X - positive value
 * @param min value for X - negative value
 * @param max value for Y - positive value
 * @param min value for Y - negative value
 * @return Nothing
 */
void Calibration::setQuadraticZoneCalibrated(int &max_axis_val_x, int &min_axis_val_x, int &max_axis_val_y, int &min_axis_val_y) {

    if (max_axis_val_x > abs(min_axis_val_x)) max_axis_val_x = abs(min_axis_val_x);
    else min_axis_val_x = -(max_axis_val_x);

    if (max_axis_val_y > abs(min_axis_val_y)) max_axis_val_y = abs(min_axis_val_y);
    else min_axis_val_y = -(max_axis_val_y);
}

/**
 * @brief Moves deadzone position after changing center position of axes
 * @return Moved deadzone position
 */
int Calibration::calibratedDeadZone(int center, int deadzone)
{
    return (center + deadzone);
}


int Calibration::fakeMapFunc(const int &x)
{
    return x;
}


void Calibration::summarizeValues(int &numbFromList, const int &mappednumb)
{
    numbFromList += mappednumb;
}


void Calibration::getMinVal(int &numbFromList, const int &mappednumb)
{
    if (numbFromList > mappednumb) numbFromList = mappednumb;
}


void Calibration::getMaxVal(int &numbFromList, const int &mappednumb)
{
    if (numbFromList < mappednumb) numbFromList = mappednumb;
}

/**
 * @brief Prepares first step of calibration - finding center
 * @return nothing
 */
void Calibration::startCalibration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool confirmed = true;

    if (stick->wasCalibrated()) {

        QMessageBox msgBox;
        msgBox.setText(tr("Calibration was saved for the preset. Do you really want to reset settings?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        switch(msgBox.exec())
        {
            case QMessageBox::Ok:
                confirmed = true;
                ui->resetBtn->setEnabled(false);
            break;

            case QMessageBox::Cancel:
                confirmed = false;
            break;

            default:
                confirmed = true;
            break;
        }
    }
    else
    {
        center_calibrated_x = 0;
        center_calibrated_y = 0;
        deadzone_calibrated_x = 0;
        deadzone_calibrated_y = 0;

        min_axis_val_x = 0;
        min_axis_val_y = 0;
        max_axis_val_x = 0;
        max_axis_val_y = 0;

        x_es_val.clear();
        y_es_val.clear();
    }


    if ((joyAxisX != nullptr) && (joyAxisY != nullptr) && confirmed) {

        center_calibrated_x = 0;
        center_calibrated_y = 0;
        deadzone_calibrated_x = 0;
        deadzone_calibrated_y = 0;

        min_axis_val_x = 0;
        min_axis_val_y = 0;
        max_axis_val_x = 0;
        max_axis_val_y = 0;

        x_es_val.clear();
        y_es_val.clear();

        stick->setCalibrationFlag(false);
        calibrated = false;

        ui->steps->setText(tr("Place the joystick in the center position.\n\nIt's the part, where often you don't have to move. Just skip it in such situation."));
        this->setWindowTitle(tr("Calibrating center"));
        ui->startButton->setText(tr("Start second step"));
        update();

        QList<int> xValues = x_es_val.values();
        QFuture<int> resX = QtConcurrent::mappedReduced(xValues, &Calibration::fakeMapFunc, &Calibration::summarizeValues);
        sumX = resX.result();

        QList<int> yValues = x_es_val.values();
        QFuture<int> resY = QtConcurrent::mappedReduced(yValues, &Calibration::fakeMapFunc, &Calibration::summarizeValues);
        sumY = resY.result();


        if ((sumX != 0) && (sumY != 0)) {

            center_calibrated_x = sumX / x_es_val.count();
            center_calibrated_y = sumY / y_es_val.count();

        } else {

            center_calibrated_x = 0;
            center_calibrated_y = 0;
        }

        QString text = QString();
        text.append(tr("\n\nCenter X: %1").arg(center_calibrated_x));
        text.append(tr("\nCenter Y: %1").arg(center_calibrated_y));
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

/**
 * @brief Prepares second step of calibration - moving into top-left corner - negative values
 * @return nothing
 */
void Calibration::startSecondStep()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joyAxisX != nullptr) && (joyAxisY != nullptr)) {

            ui->steps->setText(tr("\nPlace the joystick in the top-left corner many times"));
            this->setWindowTitle(tr("Calibrating position"));
            update();

            qDebug() << "X_ES_VAL: " << x_es_val.count(QString("-"));
            qDebug() << "Y_ES_VAL: " << y_es_val.count(QString("-"));


            if (enoughProb(x_es_val.count(QString("-")), y_es_val.count(QString("-")), QString("-"))) {

                int min_x = 0;
                int min_y = 0;

                QList<int> xValues = x_es_val.values(QString("-"));
                QFuture<int> resX = QtConcurrent::mappedReduced(xValues, &Calibration::fakeMapFunc, &Calibration::getMinVal);
                min_x = resX.result();

                QList<int> yValues = y_es_val.values(QString("-"));
                QFuture<int> resY = QtConcurrent::mappedReduced(yValues, &Calibration::fakeMapFunc, &Calibration::getMinVal);
                min_y = resY.result();

                min_axis_val_x = min_x;
                min_axis_val_y = min_y;

                QString text = ui->Informations->text();
                text.append(tr("\n\nX: %1").arg(min_axis_val_x));
                text.append(tr("\nY: %1").arg(min_axis_val_y));
                ui->Informations->setText(text);
                this->text = text;

                x_es_val.clear();
                y_es_val.clear();
                sumX = 0;
                sumY = 0;

                update();
                disconnect(ui->startButton, &QPushButton::clicked, this, nullptr);
                connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startLastStep);

            }
    }
}

/**
 * @brief Prepares third step of calibration - moving into bottom-right corner - postive values
 * @return nothing
 */
void Calibration::startLastStep()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joyAxisX != nullptr) && (joyAxisY != nullptr)) {

            ui->steps->setText(tr("\nPlace the joystick in the bottom-right corner"));
            this->setWindowTitle(tr("Calibrating position"));
            ui->startButton->setText(tr("Start final step"));
            update();

            if (enoughProb(x_es_val.count(QString("+")), y_es_val.count(QString("+")), QString("+"))) {

                int max_x = 0;
                int max_y = 0;

                QList<int> xValues = x_es_val.values(QString("+"));
                QFuture<int> resX = QtConcurrent::mappedReduced(xValues, &Calibration::fakeMapFunc, &Calibration::getMaxVal);
                max_x = resX.result();

                QList<int> yValues = y_es_val.values(QString("+"));
                QFuture<int> resY = QtConcurrent::mappedReduced(yValues, &Calibration::fakeMapFunc, &Calibration::getMaxVal);
                max_y = resY.result();

                max_axis_val_x = max_x;
                max_axis_val_y = max_y;

                QString text2 = ui->Informations->text();
                text2.append(tr("\n\nX: %1").arg(max_axis_val_x));
                text2.append(tr("\nY: %1").arg(max_axis_val_y));
                ui->Informations->setText(text2);
                this->text = text2;
                update();

                setQuadraticZoneCalibrated(max_axis_val_x, min_axis_val_x, max_axis_val_y, min_axis_val_y);

                deadzone_calibrated_x = (max_axis_val_y + max_axis_val_x)/4;
                deadzone_calibrated_y = (max_axis_val_y + max_axis_val_x)/4;

                QString text3 = ui->Informations->text();
                text3.append(tr("\n\nrange X: %1 - %2").arg(min_axis_val_x).arg(max_axis_val_x));
                text3.append(tr("\nrange Y: %1 - %2").arg(min_axis_val_y).arg(max_axis_val_y));
                text3.append(tr("\n\ndeadzone X: %1").arg(deadzone_calibrated_x));
                text3.append(tr("\ndeadzone Y: %1").arg(deadzone_calibrated_y));
                ui->Informations->setText(text3);
                this->text = text3;

                if (stick != nullptr)
                {
                    ui->saveBtn->setEnabled(true);
                }

                ui->steps->setText(tr("\n---Calibration done!---\n"));
                ui->startButton->setText(tr("Start calibration"));
                this->setWindowTitle(tr("Calibration"));
                update();

                x_es_val.clear();
                y_es_val.clear();
                sumX = 0;
                sumY = 0;

                disconnect(ui->startButton, &QPushButton::clicked, this, nullptr);
                connect(ui->startButton, &QPushButton::clicked, this, &Calibration::startCalibration);

            }
      }
}

/**
 * @brief Updates variables contents and shows message
 * @return nothing
 */
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

      ui->resetBtn->setEnabled(true);
      ui->saveBtn->setEnabled(false);

      ui->stickStatusBoxWidget->update();
      update();
      QMessageBox::information(this, tr("Save"), tr("Calibration values have been saved"));
   }
}

/**
 * @brief checks whether axes were moved at least 5 times in both ways. If not, it shows a message
 * @param counts of ax X moving values
 * @param counts of ax Y moving values
 * @return if counts of values for X and Y axes were greater than 4
 */
bool Calibration::enoughProb(int x_count, int y_count, QString character)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool enough = true;

    if ((x_count < 5) || (y_count < 5)) {
        if (character == QString("-"))
        {
            enough = false;
            QMessageBox::information(this, tr("Dead zone calibration"), tr("You have to move axes to the top-left corner at least five times."));
        }
        else if (character == QString("+"))
        {
            enough = false;
            QMessageBox::information(this, tr("Dead zone calibration"), tr("You have to move axes to the bottom-right corner at least five times."));
        }
    }

    return enough;
}

/**
 * @brief it's a slot of moving ax Y. Counts positive and negative values for later comparisions
 * @param place for sign "+" or "-". Depends on we want to find max value or min value for ax
 * @param list of moving ax values in positive and negative ways
 * @return min value if sign was "-" or max value if sign was "+"
 */
int Calibration::chooseMinMax(QString min_max_sign, QList<int> ax_values)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int min_max = 0;

    foreach(int val, ax_values)
    {
        if (min_max_sign == QString("+"))
        {
            if (min_max < val) min_max = val;
        }
        else
        {
            if (min_max > val) min_max = val;
        }
    }

    return min_max;
}

/**
 * @brief it's a slot of moving ax X. Counts positive and negative values for later comparisions
 * @param value of moving ax
 * @return nothing
 */
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

/**
 * @brief it's a slot of moving ax Y. Counts positive and negative values for later comparisions
 * @param value of moving ax
 * @return nothing
 */
void Calibration::checkY(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value > 0) {

        if (y_es_val.count(QString("+")) <= 100)
            y_es_val.insert(QString("+"), value);

    } else if (value < 0) {

        if (y_es_val.count(QString("-")) <= 100)
            y_es_val.insert(QString("-"), value);
    }

    axisBarY->setValue(value);
    update();
}

/**
 * @brief Refreshes list of sticks, which is below input devices list
 * @return nothing
 */
void Calibration::updateAxesBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->axesBox->clear();
    QList<JoyControlStick*> sticksList = currentJoystick->getActiveSetJoystick()->getSticks().values();
    QListIterator<JoyControlStick*> currStick(sticksList);

    while (currStick.hasNext())
    {
        ui->axesBox->addItem(currStick.next()->getPartialName());
    }

    update();
}

/**
 * @brief Initializes widget for moving axes (animations) and changes storing data for variables
 * @return nothing
 */
void Calibration::createAxesConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (ui->saveBtn->isEnabled()) {

        QMessageBox msgBox;
        msgBox.setText(tr("Do you want to save calibration of current axis?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        switch(msgBox.exec())
        {
            case QMessageBox::Yes:
                saveSettings();
            break;

            case QMessageBox::No:
            break;

            default:
            break;

        }
    }

    while(!ui->progressBarsLayout->isEmpty()) {

        QLayout *hb = ui->progressBarsLayout->takeAt(0)->layout();

        while(!hb->isEmpty()) {

            QWidget *w = hb->takeAt(0)->widget();
            delete w;
        }

        delete hb;
    }

    update();

    QPointer<JoyControlStick> controlstick = currentJoystick->getActiveSetJoystick()->getJoyStick(ui->axesBox->currentIndex());
    this->stick = controlstick.data();

    center_calibrated_x = controlstick->getAxisX()->getAxisCenterCal();
    center_calibrated_y = controlstick->getAxisY()->getAxisCenterCal();

    deadzone_calibrated_x = controlstick->getAxisX()->getDeadZone();
    deadzone_calibrated_y = controlstick->getAxisY()->getDeadZone();

    min_axis_val_x = controlstick->getAxisX()->getAxisMinCal();
    min_axis_val_y = controlstick->getAxisY()->getAxisMinCal();

    max_axis_val_x = controlstick->getAxisX()->getAxisMaxCal();
    max_axis_val_y = controlstick->getAxisY()->getAxisMaxCal();

    calibrated = controlstick->wasCalibrated();
    text = controlstick->getCalibrationSummary();

    if (calibrated) ui->resetBtn->setEnabled(true);
    else ui->resetBtn->setEnabled(false);

    controlstick.data()->getModifierButton()->establishPropertyUpdatedConnections();
    helper.moveToThread(controlstick.data()->thread());
    ui->stickStatusBoxWidget->setStick(controlstick.data());
    ui->stickStatusBoxWidget->update();
    setProgressBars(controlstick.data());

    update();

    if (controlstick.isNull()) controlstick.clear();
}

/**
 * @brief Creates progress bars for axes and creates connections
 * @param pointer to stick
 * @return nothing
 */
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

            axisLabel->setText(tr("Axis %1").arg(joyAxisX->getRealJoyIndex()));
            axisLabel2->setText(tr("Axis %1").arg(joyAxisY->getRealJoyIndex()));

            axisBarX = new QProgressBar();
            axisBarY = new QProgressBar();

            axisBarX->setMinimum(GlobalVariables::JoyAxis::AXISMIN);
            axisBarX->setMaximum(GlobalVariables::JoyAxis::AXISMAX);

            axisBarX->setFormat("%v");
            axisBarX->setValue(joyAxisX->getCurrentRawValue());

            axisBarY->setMinimum(GlobalVariables::JoyAxis::AXISMIN);
            axisBarY->setMaximum(GlobalVariables::JoyAxis::AXISMAX);
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

/**
 * @brief Creates progress bars for axes and creates connections
 * @param device number
 * @param joystick number
 * @param stick number
 * @return nothing
 */
void Calibration::setProgressBars(int setJoyNr, int stickNr)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

        JoyControlStick* controlstick = currentJoystick->getActiveSetJoystick()->getJoyStick(stickNr);
        //helper.moveToThread(controlstick->thread());

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

            axisLabel->setText(tr("Axis %1").arg(joyAxisX->getRealJoyIndex()));
            axisLabel2->setText(tr("Axis %1").arg(joyAxisY->getRealJoyIndex()));

            axisBarX = new QProgressBar();
            axisBarY = new QProgressBar();

            axisBarX->setMinimum(GlobalVariables::JoyAxis::AXISMIN);
            axisBarX->setMaximum(GlobalVariables::JoyAxis::AXISMAX);

            axisBarX->setFormat("%v");
            axisBarX->setValue(joyAxisX->getCurrentRawValue());

            axisBarY->setMinimum(GlobalVariables::JoyAxis::AXISMIN);
            axisBarY->setMaximum(GlobalVariables::JoyAxis::AXISMAX);
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
