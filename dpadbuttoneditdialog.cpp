#include <QDebug>

#include "dpadbuttoneditdialog.h"
#include "ui_dpadbuttoneditdialog.h"

DPadButtonEditDialog::DPadButtonEditDialog(JoyDPadButton *button, QWidget *parent) :
    ButtonEditDialog(button, parent)
{
}

//DPadButtonEditDialog::~DPadButtonEditDialog()
//{
//    delete ui;
//}

/*
DPadButtonEditDialog::DPadButtonEditDialog(JoyDPadButton *button, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DPadButtonEditDialog)
{
    ui->setupUi(this);

    this->button = button;
    this->setWindowTitle(QString("Set ").append(button->getPartialName()));

    isEditing = false;
    pendingMouseCode = 0;

    if (button->getMouse() > 0)
    {
        pendingMouseCode = button->getMouse();
    }

    QString currentlabel = QString ("Click below to change key for ");
    ui->label->setText(currentlabel.append(button->getDirectionName()));

    if (button->getKey() >= 0)
    {
        ui->pushButton->setText(keycodeToKey(button->getKey()).toUpper());
    }
    else if (button->getMouse())
    {
        ui->pushButton->setText(QString("Mouse %1").arg(button->getMouse()));
    }

    if (button->getToggleState())
    {
        ui->checkBox_2->setChecked(true);
    }

    if (button->getTurboInterval() > 0.0)
    {
        ui->checkBox->setChecked(true);
    }

    connect (ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonChanges()));
    connect (ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect (ui->pushButton, SIGNAL(clicked()), this, SLOT(changeDialogText()));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(changeDialogText(bool)));
    connect(ui->pushButton, SIGNAL(mouseCode(int)), this, SLOT(changeMouseSetting(int)));
}

DPadButtonEditDialog::~DPadButtonEditDialog()
{
    delete ui;
}

void DPadButtonEditDialog::saveButtonChanges()
{
    button->reset ();

    if (!pendingMouseCode && ui->pushButton->text() != "")
    {
        int keycode = keyToKeycode(ui->pushButton->text());
        if (keycode >= 0)
        {
            button->setKey(keycode);
        }
    }
    else if (pendingMouseCode > 0)
    {
        button->setMouse(pendingMouseCode);
    }


    if (ui->checkBox->isChecked())
    {
        button->setTurboInterval(500);
    }

    if (ui->checkBox_2->isChecked())
    {
        button->setToggle(true);
    }

    //button->joyEvent(false);
    this->close();
}

void DPadButtonEditDialog::changeDialogText(bool edited)
{
    isEditing = !isEditing;

    if (edited)
    {
        QString label = QString ("Click below to change key for ");
        label = label.append (button->getName());
        ui->label->setText(label);
    }
    else
    {
        if (isEditing)
        {
            QString label = QString ("Choose a new key or mouse button for ");
            label = label.append(button->getName());
            ui->label->setText(label);
        }
    }
}

void DPadButtonEditDialog::changeMouseSetting(int mousecode)
{
    pendingMouseCode = mousecode;
}
*/
