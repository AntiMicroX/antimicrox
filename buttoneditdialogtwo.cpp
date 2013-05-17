#include "buttoneditdialogtwo.h"
#include "ui_buttoneditdialogtwo.h"

ButtonEditDialogTwo::ButtonEditDialogTwo(JoyButton *button, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ButtonEditDialogTwo)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    this->button = button;
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget = new VirtualKeyboardMouseWidget(button, this);

    ui->verticalLayout->insertWidget(0, ui->virtualKeyMouseTabWidget);
    ui->slotSummaryLabel->setText(button->getSlotsString());
    setWindowTitle(QString("Set ").append(button->getPartialName()));

    ui->toggleCheckBox->setChecked(button->getToggleState());
    ui->turboCheckBox->setChecked(button->isUsingTurbo());

    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), this, SLOT(refreshSlotSummaryLabel()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionFinished()), this, SLOT(close()));

    connect(ui->toggleCheckBox, SIGNAL(clicked()), this, SLOT(changeToggleSetting()));
    connect(ui->turboCheckBox, SIGNAL(clicked()), this, SLOT(changeTurboSetting()));
    connect(ui->advancedPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedDialog()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardAdvancedSignalConnections()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseAdvancedSignalConnections()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(int)), this, SLOT(createTempSlot(int)));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckBox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshSlotSummaryLabel()));
}

ButtonEditDialogTwo::~ButtonEditDialogTwo()
{
    delete ui;
}

void ButtonEditDialogTwo::refreshSlotSummaryLabel()
{
    ui->slotSummaryLabel->setText(button->getSlotsString());
}

void ButtonEditDialogTwo::changeToggleSetting()
{
    button->setToggle(ui->toggleCheckBox->isChecked());
}

void ButtonEditDialogTwo::changeTurboSetting()
{
    button->setUseTurbo(ui->turboCheckBox->isChecked());
}

void ButtonEditDialogTwo::openAdvancedDialog(){
    ButtonTempConfig *tempconfig = new ButtonTempConfig(button, this);
    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(button, this);
    dialog->show();

    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardSingleSignalConnections()));
    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseSignalConnections()));
    connect(dialog, SIGNAL(turboButtonEnabledChange(bool)), this, SLOT(setTurboButtonEnabled(bool)));
    connect(dialog, SIGNAL(finished(int)), tempconfig, SLOT(deleteLater()));

    connect(this, SIGNAL(sendTempSlotToAdvanced(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), dialog, SLOT(clearAllSlots()));

    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    emit advancedDialogOpened();
}

void ButtonEditDialogTwo::createTempSlot(int keycode)
{
    JoyButtonSlot *slot = new JoyButtonSlot(keycode, JoyButtonSlot::JoyKeyboard, this);
    emit sendTempSlotToAdvanced(slot);
}

void ButtonEditDialogTwo::checkTurboSetting(bool state)
{
    if (button->containsSequence())
    {
        ui->turboCheckBox->setChecked(false);
        ui->turboCheckBox->setEnabled(false);
    }
    else
    {
        ui->turboCheckBox->setChecked(state);
        ui->turboCheckBox->setEnabled(true);
    }

    button->setUseTurbo(state);
}

void ButtonEditDialogTwo::setTurboButtonEnabled(bool state)
{
    ui->turboCheckBox->setEnabled(state);
}
