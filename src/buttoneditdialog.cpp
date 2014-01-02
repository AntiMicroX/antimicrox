#include <QDebug>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include "wininfo.h"

#endif

#include "buttoneditdialog.h"
#include "ui_buttoneditdialog.h"

#include "antkeymapper.h"

ButtonEditDialog::ButtonEditDialog(JoyButton *button, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::ButtonEditDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    setMinimumHeight(460);
#endif

    setAttribute(Qt::WA_DeleteOnClose);

    ignoreRelease = false;

    this->button = button;
    ui->virtualKeyMouseTabWidget->hide();
    ui->virtualKeyMouseTabWidget->deleteLater();
    ui->virtualKeyMouseTabWidget = new VirtualKeyboardMouseWidget(button, this);
    //ui->virtualKeyMouseTabWidget->setFocus();

    ui->verticalLayout->insertWidget(1, ui->virtualKeyMouseTabWidget);
    ui->slotSummaryLabel->setText(button->getSlotsString());
    updateWindowTitleButtonName();

    ui->toggleCheckBox->setChecked(button->getToggleState());
    ui->turboCheckBox->setChecked(button->isUsingTurbo());

    if (!button->getActionName().isEmpty())
    {
        ui->actionNameLineEdit->setText(button->getActionName());
    }

    if (!button->getButtonName().isEmpty())
    {
        ui->buttonNameLineEdit->setText(button->getButtonName());
    }

    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), this, SLOT(refreshSlotSummaryLabel()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionFinished()), this, SLOT(close()));

    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), this, SLOT(processSlotAssignment(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(clearButtonSlots()));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(sendSelectionFinished()));
    connect(this, SIGNAL(selectionFinished()), this, SLOT(close()));

    connect(ui->toggleCheckBox, SIGNAL(clicked()), this, SLOT(changeToggleSetting()));
    connect(ui->turboCheckBox, SIGNAL(clicked()), this, SLOT(changeTurboSetting()));
    connect(ui->advancedPushButton, SIGNAL(clicked()), this, SLOT(openAdvancedDialog()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardAdvancedSignalConnections()));
    connect(this, SIGNAL(advancedDialogOpened()), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseAdvancedSignalConnections()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(int)), this, SLOT(createTempSlot(int)));

    connect(ui->actionNameLineEdit, SIGNAL(textEdited(QString)), button, SLOT(setActionName(QString)));
    connect(ui->buttonNameLineEdit, SIGNAL(textEdited(QString)), button, SLOT(setButtonName(QString)));

    connect(button, SIGNAL(toggleChanged(bool)), ui->toggleCheckBox, SLOT(setChecked(bool)));
    connect(button, SIGNAL(turboChanged(bool)), this, SLOT(checkTurboSetting(bool)));
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshSlotSummaryLabel()));
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(updateWindowTitleButtonName()));
}

ButtonEditDialog::~ButtonEditDialog()
{
    delete ui;
}

void ButtonEditDialog::keyPressEvent(QKeyEvent *event)
{
    bool ignore = false;
    // Ignore the following keys that might
    // trigger an event in QDialog::keyPressEvent
    switch(event->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Right:
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            ignore = true;
            break;
        }
    }

    if (!ignore)
    {
        QDialog::keyPressEvent(event);
    }
}

void ButtonEditDialog::keyReleaseEvent(QKeyEvent *event)
{
    if (ui->actionNameLineEdit->hasFocus() || ui->buttonNameLineEdit->hasFocus())
    {
        QDialog::keyReleaseEvent(event);
    }
    else if (ui->virtualKeyMouseTabWidget->isKeyboardTabVisible())
    {
        int controlcode = event->nativeScanCode();
        int virtualactual = event->nativeVirtualKey();

#ifdef Q_OS_WIN
        int finalvirtual = WinInfo::correctVirtualKey(controlcode, virtualactual);
        int checkalias = AntKeyMapper::returnQtKey(virtualactual);

#else
        int checkalias = AntKeyMapper::returnQtKey(virtualactual);
#endif

        if (checkalias <= 0)
        {
            controlcode = 0;
        }

#ifndef Q_OS_WIN
        Q_UNUSED(virtualactual);
#endif


        if (!ignoreRelease)
        {
            if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_X)
            {
                controlcode = 0;
                ignoreRelease = true;
                emit selectionCleared();
            }
            else if (controlcode < 0)
            {
                controlcode = 0;
            }
        }
        else
        {
            controlcode = 0;
            ignoreRelease = false;
        }


        if (controlcode > 0)
        {
#if defined (Q_OS_UNIX)
            JoyButtonSlot *tempslot = new JoyButtonSlot(virtualactual, JoyButtonSlot::JoyKeyboard, this);

#elif defined (Q_OS_WIN)
            JoyButtonSlot *tempslot = new JoyButtonSlot(finalvirtual, JoyButtonSlot::JoyKeyboard, this);

#endif
            emit keyGrabbed(tempslot);
        }
    }
    else
    {
        QDialog::keyReleaseEvent(event);
    }
}

void ButtonEditDialog::refreshSlotSummaryLabel()
{
    ui->slotSummaryLabel->setText(button->getSlotsString().replace("&", "&&"));
}

void ButtonEditDialog::changeToggleSetting()
{
    button->setToggle(ui->toggleCheckBox->isChecked());
}

void ButtonEditDialog::changeTurboSetting()
{
    button->setUseTurbo(ui->turboCheckBox->isChecked());
}

void ButtonEditDialog::openAdvancedDialog(){
    ui->advancedPushButton->setEnabled(false);

    AdvanceButtonDialog *dialog = new AdvanceButtonDialog(button, this);
    dialog->show();

    // Disconnect event to allow for placing slot to AdvanceButtonDialog
    disconnect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), 0, 0);
    disconnect(this, SIGNAL(selectionCleared()), 0, 0);
    disconnect(this, SIGNAL(selectionFinished()), 0, 0);

    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualKeyboardSingleSignalConnections()));
    connect(dialog, SIGNAL(finished(int)), ui->virtualKeyMouseTabWidget, SLOT(establishVirtualMouseSignalConnections()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(closedAdvancedDialog()));
    connect(dialog, SIGNAL(turboButtonEnabledChange(bool)), this, SLOT(setTurboButtonEnabled(bool)));

    connect(this, SIGNAL(sendTempSlotToAdvanced(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), dialog, SLOT(clearAllSlots()));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionMade(JoyButtonSlot*)), dialog, SLOT(placeNewSlot(JoyButtonSlot*)));
    connect(ui->virtualKeyMouseTabWidget, SIGNAL(selectionCleared()), dialog, SLOT(clearAllSlots()));

    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    emit advancedDialogOpened();
}

void ButtonEditDialog::createTempSlot(int keycode)
{
    JoyButtonSlot *slot = new JoyButtonSlot(keycode, JoyButtonSlot::JoyKeyboard, this);
    emit sendTempSlotToAdvanced(slot);
}

void ButtonEditDialog::checkTurboSetting(bool state)
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

void ButtonEditDialog::setTurboButtonEnabled(bool state)
{
    ui->turboCheckBox->setEnabled(state);
}

void ButtonEditDialog::closedAdvancedDialog()
{
    ui->advancedPushButton->setEnabled(true);

    // Re-connect previously disconnected event
    connect(this, SIGNAL(keyGrabbed(JoyButtonSlot*)), this, SLOT(processSlotAssignment(JoyButtonSlot*)));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(clearButtonSlots()));
    connect(this, SIGNAL(selectionCleared()), this, SLOT(sendSelectionFinished()));
    connect(this, SIGNAL(selectionFinished()), this, SLOT(close()));
}

void ButtonEditDialog::processSlotAssignment(JoyButtonSlot *tempslot)
{
    button->clearSlotsEventReset();
    button->setAssignedSlot(tempslot->getSlotCode(), tempslot->getSlotMode());
    this->close();
}

void ButtonEditDialog::clearButtonSlots()
{
    button->clearSlotsEventReset();
}

void ButtonEditDialog::sendSelectionFinished()
{
    emit selectionFinished();
}

void ButtonEditDialog::updateWindowTitleButtonName()
{
    QString temp = QString(tr("Set")).append(" ");
    if (!button->getButtonName().isEmpty())
    {
        temp.append(button->getPartialName(true));
    }
    else
    {
        temp.append(button->getPartialName());
    }
    setWindowTitle(temp);
}
