#include <QDebug>

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"
#include "advancebuttondialog.h"

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
    defaultLabel = defaultLabel.append(button->getPartialName());
    ui->label->setText(defaultLabel);

    ui->checkBox->setChecked(button->isUsingTurbo());
    ui->checkBox_2->setChecked(button->getToggleState());

    tempconfig = new ButtonTempConfig(button);

    tempconfig->assignments->clear();
    QListIterator<JoyButtonSlot*> iter(*(button->getAssignedSlots()));
    bool updatedPushValue = false;
    while (iter.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter.next();
        if (!updatedPushValue)
        {
            ui->pushButton->setValue(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
            updatedPushValue = !updatedPushValue;
        }

        tempconfig->assignments->append(tempbuttonslot);
    }

    updateFromTempConfig();

    connect (ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonChanges()));
    connect (ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect (ui->pushButton, SIGNAL(grabStarted()), this, SLOT(changeDialogText()));

    connect(ui->pushButton, SIGNAL(grabStarted()), this, SLOT(disableDialogButtons()));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(changeDialogText(bool)));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(enableDialogButtons()));
    connect(ui->pushButton, SIGNAL(grabFinished(bool)), this, SLOT(singleAssignmentForTempConfig(bool)));
    connect(ui->advancedButton, SIGNAL(clicked()), this, SLOT(openAdvancedDialog()));
}

ButtonEditDialog::~ButtonEditDialog()
{
    delete ui;
    delete tempconfig;
}

void ButtonEditDialog::saveButtonChanges()
{
    bool pressed = button->getButtonState();
    if (pressed)
    {
        button->joyEvent(false);
    }
    JoyButton::SetChangeCondition oldCondition = button->getChangeSetCondition();

    button->reset ();

    updateTempConfigState();

    QListIterator<JoyButtonSlot*> iter(*(tempconfig->assignments));
    while (iter.hasNext())
    {
        JoyButtonSlot *tempbuttonslot = iter.next();
        button->setAssignedSlot(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
    }

    button->setTurboInterval(tempconfig->turboInterval);

    if (ui->checkBox->isChecked())
    {
        button->setUseTurbo(true);
    }

    if (ui->checkBox_2->isChecked())
    {
        button->setToggle(true);
    }

    button->setMouseSpeedX(tempconfig->mouseSpeedX);
    button->setMouseSpeedY(tempconfig->mouseSpeedY);

    if (tempconfig->setSelection > -1 && tempconfig->setSelectionCondition != JoyButton::SetChangeDisabled)
    {
        // Revert old set condition before entering new set condition.
        // Also, do not emit signals on first change
        button->setChangeSetCondition(oldCondition, true);

        button->setChangeSetSelection(tempconfig->setSelection);
        button->setChangeSetCondition(tempconfig->setSelectionCondition);
    }
    else
    {
        button->setChangeSetSelection(-1);
        button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    }

    if (pressed)
    {
        button->joyEvent(pressed);
    }

    this->close();
}

void ButtonEditDialog::changeDialogText(bool edited)
{
    isEditing = !isEditing;

    if (isEditing && edited)
    {
        ui->label->setText(defaultLabel);
        isEditing = false;
    }
    else if (isEditing)
    {
        QString currentlabel = QString ("Choose a new key or mouse button for ");
        currentlabel = currentlabel.append(button->getPartialName());
        currentlabel = currentlabel.append("\n(Ctrl+X to reset key)");
        ui->label->setText(currentlabel);
    }
    else
    {
        ui->label->setText(defaultLabel);
    }
}

void ButtonEditDialog::enableDialogButtons()
{
    ui->buttonBox->setEnabled(true);
    ui->advancedButton->setEnabled(true);
}

void ButtonEditDialog::disableDialogButtons()
{
    ui->buttonBox->setEnabled(false);
    ui->advancedButton->setEnabled(false);
}

void ButtonEditDialog::openAdvancedDialog()
{
    updateTempConfigState();

    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(tempconfig);
    dialog->show();

    connect(dialog, SIGNAL(accepted()), this, SLOT(updateFromTempConfig()));
}

void ButtonEditDialog::updateFromTempConfig()
{
    ui->checkBox->setChecked(tempconfig->turbo);
    ui->checkBox_2->setChecked(tempconfig->toggle);

    int slotcount = tempconfig->assignments->count();
    ui->pushButton->setText(tempconfig->getSlotsSummary());
    if (slotcount > 0)
    {
        JoyButtonSlot *tempbuttonslot = tempconfig->assignments->at(0);
        ui->pushButton->setValue(tempbuttonslot->getSlotCode(), tempbuttonslot->getSlotMode());
    }
    else
    {
        ui->pushButton->setValue(0);
    }

    if (tempconfig->containsSequence())
    {
        ui->checkBox->setEnabled(false);
    }
    else
    {
        ui->checkBox->setEnabled(true);
    }
}

void ButtonEditDialog::updateTempConfigState()
{
    tempconfig->turbo = ui->checkBox->isChecked();
    tempconfig->toggle = ui->checkBox_2->isChecked();
}

void ButtonEditDialog::singleAssignmentForTempConfig(bool edited)
{
    if (edited)
    {
        JoyButtonSlot *buttonslot = ui->pushButton->getValue();
        JoyButtonSlot *newbuttonslot = new JoyButtonSlot(buttonslot->getSlotCode(), buttonslot->getSlotMode());
        tempconfig->assignments->clear();
        tempconfig->assignments->append(newbuttonslot);
        ui->checkBox->setEnabled(true);
    }
}
