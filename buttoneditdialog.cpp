#include <QDebug>

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"
#include "event.h"

ButtonEditDialog::ButtonEditDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->button = button;
    this->setWindowTitle(QString("Set ").append(button->getPartialName()));
    isEditing = false;

    defaultLabel = QString ("Click below to change key or mouse button for ");
    QString currentlabel = defaultLabel;
    ui->label->setText(currentlabel.append(button->getPartialName()));

    ui->pushButton->setText("[NO KEY]");
    if (button->getKey() > 0 && button->getKey() <= MOUSE_OFFSET)
    {
        ui->pushButton->setText(keycodeToKey(button->getKey()).toUpper());
        ui->pushButton->setValue(button->getKey());
    }
    else if (button->getMouse() > 0)
    {
        ui->pushButton->setText(QString("Mouse %1").arg(button->getMouse()));
        ui->pushButton->setValue(button->getMouse() + MOUSE_OFFSET);
    }

    if (button->getToggleState())
    {
        ui->checkBox_2->setChecked(true);
    }

    if (button->isUsingTurbo())
    {
        int interval = (int)(button->getTurboInterval() / 100);
        this->changeTurboText(interval);
        ui->checkBox->setChecked(true);
        ui->horizontalSlider->setEnabled(true);
        ui->horizontalSlider->setValue(interval);
    }
    else
    {
        int interval = (int)(button->getTurboInterval() / 100);
        this->changeTurboText(interval);
        ui->checkBox->setChecked(false);
        ui->horizontalSlider->setValue(interval);
    }

    connect (ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonChanges()));
    connect (ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect (ui->pushButton, SIGNAL(clicked()), this, SLOT(changeDialogText()));
    connect(ui->checkBox, SIGNAL(clicked(bool)), ui->horizontalSlider, SLOT(setEnabled(bool)));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(changeDialogText(bool)));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(enableDialogButtons()));
    connect(ui->pushButton, SIGNAL(grabStarted()), this, SLOT(disableDialogButtons()));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeTurboText(int)));
    //connect(ui->pushButton, SIGNAL(mouseCode(int)), this, SLOT(changeMouseSetting(int)));
}

ButtonEditDialog::~ButtonEditDialog()
{
    delete ui;
}

void ButtonEditDialog::saveButtonChanges()
{
    button->reset ();

    if (ui->pushButton->getValue() >= 0)
    {
        int keycode = ui->pushButton->getValue();
        if (keycode == 0)
        {
            button->setKey(keycode);
        }
        else if (keycode <= MOUSE_OFFSET)
        {
            button->setKey(keycode);
        }
        else
        {
            button->setMouse(keycode - MOUSE_OFFSET);
            button->setUseMouse(true);
        }
    }

    int turboInterval = ui->horizontalSlider->value() * 100;
    button->setTurboInterval(turboInterval);

    if (ui->checkBox->isChecked())
    {
        button->setUseTurbo(true);
    }

    if (ui->checkBox_2->isChecked())
    {
        button->setToggle(true);
    }

    //button->joyEvent(false);
    this->close();
}

void ButtonEditDialog::changeDialogText(bool edited)
{
    isEditing = !isEditing;

    if (edited)
    {
        QString label = defaultLabel;
        label = label.append(button->getPartialName());
        ui->label->setText(label);
    }
    else
    {
        if (isEditing)
        {
            QString label = QString ("Choose a new key or mouse button for ");
            label = label.append(button->getPartialName());
            ui->label->setText(label);
        }
    }
}

void ButtonEditDialog::changeTurboText(int value)
{
    if (value == 0)
    {
        value = 1;
    }
    double clicks = 100 / (value * 10.0);
    QString labeltext = QString(QString::number(clicks, 'g', 2)).append("/sec.");
    ui->label_3->setText(labeltext);
}

void ButtonEditDialog::enableDialogButtons()
{
    ui->buttonBox->setEnabled(true);
}

void ButtonEditDialog::disableDialogButtons()
{
    ui->buttonBox->setEnabled(false);
}
