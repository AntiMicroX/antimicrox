//#include <QDebug>
#include <QList>
#include <QListIterator>
#include <QTableWidgetItem>
#include <QAbstractItemModel>
#include <QModelIndexList>
#include <QVariant>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>

#include "gamecontrollermappingdialog.h"
#include "ui_gamecontrollermappingdialog.h"

static QHash<int, QString> initAliases()
{
    QHash<int, QString> temp;
    temp.insert(0, "a");
    temp.insert(1, "b");
    temp.insert(2, "x");
    temp.insert(3, "y");
    temp.insert(4, "back");
    temp.insert(5, "start");
    temp.insert(6, "guide");
    temp.insert(7, "leftshoulder");
    temp.insert(8, "rightshoulder");
    temp.insert(9, "leftstick");
    temp.insert(10, "rightstick");
    temp.insert(11, "leftx");
    temp.insert(12, "lefty");
    temp.insert(13, "rightx");
    temp.insert(14, "righty");
    temp.insert(15, "lefttrigger");
    temp.insert(16, "righttrigger");
    temp.insert(17, "dpup");
    temp.insert(18, "dpleft");
    temp.insert(19, "dpdown");
    temp.insert(20, "dpright");
    return temp;
}

static QHash<SDL_GameControllerButton, int> initButtonPlacement()
{
    QHash<SDL_GameControllerButton, int> temp;
    temp.insert(SDL_CONTROLLER_BUTTON_A, 0);
    temp.insert(SDL_CONTROLLER_BUTTON_B, 1);
    temp.insert(SDL_CONTROLLER_BUTTON_X, 2);
    temp.insert(SDL_CONTROLLER_BUTTON_Y, 3);
    temp.insert(SDL_CONTROLLER_BUTTON_BACK, 4);
    temp.insert(SDL_CONTROLLER_BUTTON_START, 5);
    temp.insert(SDL_CONTROLLER_BUTTON_GUIDE, 6);
    temp.insert(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, 7);
    temp.insert(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 8);
    temp.insert(SDL_CONTROLLER_BUTTON_LEFTSTICK, 9);
    temp.insert(SDL_CONTROLLER_BUTTON_RIGHTSTICK, 10);
    temp.insert(SDL_CONTROLLER_BUTTON_DPAD_UP, 17);
    temp.insert(SDL_CONTROLLER_BUTTON_DPAD_LEFT, 18);
    temp.insert(SDL_CONTROLLER_BUTTON_DPAD_DOWN, 19);
    temp.insert(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 20);
    return temp;
}

static QHash<SDL_GameControllerAxis, int> initAxisPlacement()
{
    QHash<SDL_GameControllerAxis, int> temp;
    temp.insert(SDL_CONTROLLER_AXIS_LEFTX, 11);
    temp.insert(SDL_CONTROLLER_AXIS_LEFTY, 12);
    temp.insert(SDL_CONTROLLER_AXIS_RIGHTX, 13);
    temp.insert(SDL_CONTROLLER_AXIS_RIGHTY, 14);
    temp.insert(SDL_CONTROLLER_AXIS_TRIGGERLEFT, 15);
    temp.insert(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 16);
    return temp;
}

QHash<int, QString> GameControllerMappingDialog::tempaliases = initAliases();

QHash<SDL_GameControllerButton, int> GameControllerMappingDialog::buttonPlacement =  initButtonPlacement();
QHash<SDL_GameControllerAxis, int> GameControllerMappingDialog::axisPlacement = initAxisPlacement();

GameControllerMappingDialog::GameControllerMappingDialog(InputDevice *device, AntiMicroSettings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameControllerMappingDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    buttonGrabs = 0;

    this->device = device;
    this->settings = settings;

    device->getActiveSetJoystick()->setIgnoreEventState(true);
    device->getActiveSetJoystick()->release();

    device->getActiveSetJoystick()->currentAxesDeadZones(&originalAxesDeadZones);
    device->getActiveSetJoystick()->raiseAxesDeadZones();

    GameController *controller = qobject_cast<GameController*>(device);
    if (controller)
    {
        populateGameControllerBindings(controller);
        ui->mappingStringPlainTextEdit->document()->setPlainText(generateSDLMappingString());
    }

    QString tempWindowTitle = QString(tr("Game Controller Mapping (%1) (#%2)")).arg(device->getSDLName())
                                     .arg(device->getRealJoyNumber());
    setWindowTitle(tempWindowTitle);

    enableDeviceConnections();

    ui->buttonMappingTableWidget->setCurrentCell(0, 0);

    ui->axisDeadZoneComboBox->clear();
    populateAxisDeadZoneComboBox();

    int index = ui->axisDeadZoneComboBox->findData(20000);
    if (index != -1)
    {
        ui->axisDeadZoneComboBox->setCurrentIndex(index);
    }

    connect(device, SIGNAL(destroyed()), this, SLOT(obliterate()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveChanges()));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(discardMapping(QAbstractButton*)));
    connect(ui->buttonMappingTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(changeButtonDisplay()));
    connect(ui->axisDeadZoneComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeAxisDeadZone(int)));
    connect(this, SIGNAL(finished(int)), this, SLOT(enableButtonEvents(int)));
}

GameControllerMappingDialog::~GameControllerMappingDialog()
{
    delete ui;
}

void GameControllerMappingDialog::buttonAssign(int buttonindex)
{
    // Only perform assignment if no other control is currently active.
    if (ui->buttonMappingTableWidget->currentRow() > -1 && buttonGrabs == 0)
    {
        QTableWidgetItem* item = ui->buttonMappingTableWidget->currentItem();
        int column = ui->buttonMappingTableWidget->currentColumn();
        int row = ui->buttonMappingTableWidget->currentRow();

        if (!item)
        {
            item = new QTableWidgetItem(QString("Button %1").arg(buttonindex+1));
            ui->buttonMappingTableWidget->setItem(row, column, item);
        }

        QList<QVariant> templist;
        templist.append(QVariant(0));
        templist.append(QVariant(buttonindex));
        QAbstractItemModel *model = ui->buttonMappingTableWidget->model();
        QModelIndexList matchlist = model->match(model->index(0,0), Qt::UserRole, templist, 1, Qt::MatchExactly);
        foreach (const QModelIndex &index, matchlist) {
            QTableWidgetItem *existingItem = ui->buttonMappingTableWidget->item(index.row(), index.column());
            if (existingItem)
            {
                existingItem->setText("");
                existingItem->setData(Qt::UserRole, QVariant());
            }
        }

        QList<QVariant> tempvalue;
        tempvalue.append(QVariant(0));
        tempvalue.append(QVariant(buttonindex));

        item->setData(Qt::UserRole, tempvalue);
        item->setText(QString("Button %1").arg(buttonindex+1));

        if (row < ui->buttonMappingTableWidget->rowCount()-1)
        {
            ui->buttonMappingTableWidget->setCurrentCell(row+1, column);
        }

        ui->mappingStringPlainTextEdit->document()->setPlainText(generateSDLMappingString());
    }

    // Increment reference count.
    buttonGrabs++;
}

void GameControllerMappingDialog::axisAssign(int axis, int value)
{
    Q_UNUSED(value);

    //qDebug() << value;

    // Only perform assignment if no other control is currently active.
    if (ui->buttonMappingTableWidget->currentRow() > -1 && buttonGrabs == 0)
    {
        QTableWidgetItem* item = ui->buttonMappingTableWidget->currentItem();
        int column = ui->buttonMappingTableWidget->currentColumn();
        int row = ui->buttonMappingTableWidget->currentRow();

        if (row < 17)
        {
            if (!item)
            {
                item = new QTableWidgetItem(QString("Axis %1").arg(axis+1));
                ui->buttonMappingTableWidget->setItem(row, column, item);
            }

            QList<QVariant> templist;
            templist.append(QVariant(axis+1));
            templist.append(QVariant(0));
            QAbstractItemModel *model = ui->buttonMappingTableWidget->model();
            QModelIndexList matchlist = model->match(model->index(0,0), Qt::UserRole, templist, 1, Qt::MatchExactly);
            foreach (const QModelIndex &index, matchlist) {
                QTableWidgetItem *existingItem = ui->buttonMappingTableWidget->item(index.row(), index.column());
                if (existingItem)
                {
                    existingItem->setText("");
                    existingItem->setData(Qt::UserRole, QVariant());
                }
            }

            QList<QVariant> tempvalue;
            tempvalue.append(QVariant(axis+1));
            tempvalue.append(QVariant(0));

            item->setData(Qt::UserRole, tempvalue);
            item->setText(QString("Axis %1").arg(axis+1));

            if (row < ui->buttonMappingTableWidget->rowCount()-1)
            {
                ui->buttonMappingTableWidget->setCurrentCell(row+1, column);
            }

            ui->mappingStringPlainTextEdit->document()->setPlainText(generateSDLMappingString());
        }
    }

    // Increment reference count.
    buttonGrabs++;
}

void GameControllerMappingDialog::dpadAssign(int dpad, int buttonindex)
{
    // Only perform assignment if no other control is currently active.
    if (ui->buttonMappingTableWidget->currentRow() > -1 && buttonGrabs == 0)
    {
        if (buttonindex == JoyDPadButton::DpadUp ||
            buttonindex == JoyDPadButton::DpadDown ||
            buttonindex == JoyDPadButton::DpadLeft ||
            buttonindex == JoyDPadButton::DpadRight)
        {
            QTableWidgetItem* item = ui->buttonMappingTableWidget->currentItem();
            int column = ui->buttonMappingTableWidget->currentColumn();
            int row = ui->buttonMappingTableWidget->currentRow();

            if (row <= 10 || row >= 17)
            {
                if (!item)
                {
                    item = new QTableWidgetItem(QString("Hat %1.%2").arg(dpad+1).arg(buttonindex));
                    ui->buttonMappingTableWidget->setItem(row, column, item);
                }

                QList<QVariant> templist;
                templist.append(QVariant(-dpad-1));
                templist.append(QVariant(buttonindex));
                QAbstractItemModel *model = ui->buttonMappingTableWidget->model();
                QModelIndexList matchlist = model->match(model->index(0,0), Qt::UserRole, templist, 1, Qt::MatchExactly);
                foreach (const QModelIndex &index, matchlist) {
                    QTableWidgetItem *existingItem = ui->buttonMappingTableWidget->item(index.row(), index.column());
                    if (existingItem)
                    {
                        existingItem->setText("");
                        existingItem->setData(Qt::UserRole, QVariant());
                    }
                }

                QList<QVariant> tempvalue;
                tempvalue.append(QVariant(-dpad-1));
                tempvalue.append(QVariant(buttonindex));

                item->setData(Qt::UserRole, tempvalue);
                item->setText(QString("Hat %1.%2").arg(dpad+1).arg(buttonindex));
            }

            if (row < ui->buttonMappingTableWidget->rowCount()-1)
            {
                ui->buttonMappingTableWidget->setCurrentCell(row+1, column);
            }

            ui->mappingStringPlainTextEdit->document()->setPlainText(generateSDLMappingString());
        }
    }

    // Increment reference count.
    buttonGrabs++;
}

void GameControllerMappingDialog::saveChanges()
{
    QString mappingString = generateSDLMappingString();

    settings->setValue(QString("Mappings/").append(device->getGUIDString()), mappingString);
    settings->setValue(QString("Mappings/%1%2").arg(device->getGUIDString()).arg("Disable"), "0");
    settings->sync();
    bool displayMapping = settings->runtimeValue("DisplaySDLMapping", false).toBool();
    if (displayMapping)
    {
        QTextStream out(stdout);
        out << generateSDLMappingString();
    }

    emit mappingUpdate(mappingString, device);
}

void GameControllerMappingDialog::populateGameControllerBindings(GameController *controller)
{
    if (controller)
    {
        for (int i = 0; i < controller->getNumberButtons(); i++)
        {
            int associatedRow = buttonPlacement.value((SDL_GameControllerButton)i);
            SDL_GameControllerButtonBind bind = controller->getBindForButton(i);
            QString temptext = bindingString(bind);
            if (!temptext.isEmpty())
            {
                QList<QVariant> tempvariant = bindingValues(bind);
                QTableWidgetItem* item = new QTableWidgetItem();
                ui->buttonMappingTableWidget->setItem(associatedRow, 0, item);
                item->setText(temptext);
                item->setData(Qt::UserRole, tempvariant);
            }
        }

        for (int i = 0; i < controller->getNumberAxes(); i++)
        {
            int associatedRow = axisPlacement.value((SDL_GameControllerAxis)i);
            SDL_GameControllerButtonBind bind = controller->getBindForAxis(i);
            QString temptext = bindingString(bind);
            if (!temptext.isEmpty())
            {
                QList<QVariant> tempvariant = bindingValues(bind);
                QTableWidgetItem* item = new QTableWidgetItem();
                ui->buttonMappingTableWidget->setItem(associatedRow, 0, item);
                item->setText(temptext);
                item->setData(Qt::UserRole, tempvariant);
            }
        }
    }
}

QString GameControllerMappingDialog::bindingString(SDL_GameControllerButtonBind bind)
{
    QString temp;

    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            temp.append(QString("Button %1").arg(bind.value.button+1));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            temp.append(QString("Axis %1").arg(bind.value.axis+1));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            temp.append(QString("Hat %1.%2").arg(bind.value.hat.hat+1)
                    .arg(bind.value.hat.hat_mask));
        }
    }

    return temp;
}

QList<QVariant> GameControllerMappingDialog::bindingValues(SDL_GameControllerButtonBind bind)
{
    QList<QVariant> temp;

    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            temp.append(QVariant(0));
            temp.append(QVariant(bind.value.button));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            temp.append(QVariant(bind.value.axis+1));
            temp.append(QVariant(0));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            temp.append(QVariant(-bind.value.hat.hat-1));
            temp.append(QVariant(bind.value.hat.hat_mask));
        }
    }

    return temp;
}

void GameControllerMappingDialog::discardMapping(QAbstractButton *button)
{
    disableDeviceConnections();
    QDialogButtonBox::ButtonRole currentRole = ui->buttonBox->buttonRole(button);
    if (currentRole == QDialogButtonBox::DestructiveRole)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Discard Controller Mapping?"));
        msgBox.setText(tr("Discard mapping for this controller?\n\nIf discarded, the controller will be reverted to a joystick once you refresh all joysticks."));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

        int status = msgBox.exec();
        if (status == QMessageBox::Yes)
        {
            removeControllerMapping();
            close();
        }
        else
        {
            connect(device, SIGNAL(rawButtonClick(int)), this, SLOT(buttonAssign(int)));
            connect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(axisAssign(int,int)));
            connect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(dpadAssign(int,int)));
        }
    }
}

void GameControllerMappingDialog::removeControllerMapping()
{
    settings->beginGroup("Mappings");
    settings->remove(device->getGUIDString());
    settings->remove(QString("%1Disable").arg(device->getGUIDString()));
    settings->endGroup();
    settings->sync();
}

void GameControllerMappingDialog::enableDeviceConnections()
{
    connect(device, SIGNAL(rawButtonClick(int)), this, SLOT(buttonAssign(int)));
    connect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(axisAssign(int,int)));
    for (int i=0; i < device->getActiveSetJoystick()->getNumberAxes(); i++)
    {
        JoyAxis *temp = device->getActiveSetJoystick()->getJoyAxis(i);
        if (temp)
        {
            connect(temp, SIGNAL(moved(int)), this, SLOT(updateLastAxisLineEdit(int)));
        }
    }
    //connect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(updateLastAxisLineEdit(int, int)));
    connect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(dpadAssign(int,int)));

    connect(device, SIGNAL(rawButtonRelease(int)), this, SLOT(buttonRelease(int)));
    connect(device, SIGNAL(rawAxisReleased(int,int)), this, SLOT(axisRelease(int,int)));
    connect(device, SIGNAL(rawDPadButtonRelease(int,int)), this, SLOT(dpadRelease(int,int)));
}

void GameControllerMappingDialog::disableDeviceConnections()
{
    disconnect(device, SIGNAL(rawButtonClick(int)), this, SLOT(buttonAssign(int)));
    disconnect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(axisAssign(int,int)));
    for (int i=0; i < device->getActiveSetJoystick()->getNumberAxes(); i++)
    {
        JoyAxis *temp = device->getActiveSetJoystick()->getJoyAxis(i);
        if (temp)
        {
            disconnect(temp, SIGNAL(moved(int)), this, SLOT(updateLastAxisLineEdit(int)));
        }
    }

    disconnect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(dpadAssign(int,int)));

    disconnect(device, SIGNAL(rawButtonRelease(int)), this, SLOT(buttonRelease(int)));
    disconnect(device, SIGNAL(rawAxisReleased(int,int)), this, SLOT(axisRelease(int,int)));
    disconnect(device, SIGNAL(rawDPadButtonRelease(int,int)), this, SLOT(dpadRelease(int,int)));
}

void GameControllerMappingDialog::enableButtonEvents(int code)
{
    Q_UNUSED(code);

    //if (code == QDialogButtonBox::AcceptRole)
    //{
        device->getActiveSetJoystick()->setIgnoreEventState(false);
        device->getActiveSetJoystick()->release();
        device->getActiveSetJoystick()->setAxesDeadZones(&originalAxesDeadZones);
    //}
}

QString GameControllerMappingDialog::generateSDLMappingString()
{
    QStringList templist;
    templist.append(device->getGUIDString());
    templist.append(device->getSDLName());
    templist.append(QString("platform:").append(device->getSDLPlatform()));

    for (int i=0; i < ui->buttonMappingTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *item = ui->buttonMappingTableWidget->item(i, 0);
        if (item)
        {
            QString mapNative;
            QList<QVariant> tempassociation = item->data(Qt::UserRole).toList();
            int bindingType = tempassociation.value(0).toInt();
            if (bindingType == 0)
            {
                mapNative.append("b");
                mapNative.append(QString::number(tempassociation.value(1).toInt()));
            }
            else if (bindingType > 0)
            {
                mapNative.append("a");
                mapNative.append(QString::number(tempassociation.value(0).toInt()-1));
            }
            else if (bindingType < 0)
            {
                mapNative.append("h");
                mapNative.append(QString::number(tempassociation.value(0).toInt()+1));
                mapNative.append(".").append(QString::number(tempassociation.value(1).toInt()));
            }

            QString sdlButtonName = tempaliases.value(i);
            QString temp = QString("%1:%2").arg(sdlButtonName).arg(mapNative);
            templist.append(temp);
        }
    }

    return templist.join(",").append(",");
}

void GameControllerMappingDialog::obliterate()
{
    this->done(QDialogButtonBox::DestructiveRole);
}

void GameControllerMappingDialog::changeButtonDisplay()
{

    ui->gameControllerDisplayWidget->setActiveButton(ui->buttonMappingTableWidget->currentRow());
}

void GameControllerMappingDialog::axisRelease(int axis, int value)
{
    Q_UNUSED(axis);
    Q_UNUSED(value);

    if (buttonGrabs > 0)
    {
        buttonGrabs--;
    }
    else
    {
        buttonGrabs = 0;
    }
}

void GameControllerMappingDialog::buttonRelease(int buttonindex)
{
    Q_UNUSED(buttonindex);

    if (buttonGrabs > 0)
    {
        buttonGrabs--;
    }
    else
    {
        buttonGrabs = 0;
    }
}

void GameControllerMappingDialog::dpadRelease(int dpad, int buttonindex)
{
    Q_UNUSED(dpad);
    Q_UNUSED(buttonindex);

    if (buttonGrabs > 0)
    {
        buttonGrabs--;
    }
    else
    {
        buttonGrabs = 0;
    }
}

void GameControllerMappingDialog::populateAxisDeadZoneComboBox()
{
    for (int i=0; i < 28; i++)
    {
        unsigned int temp = (i * 1000) + 5000;
        ui->axisDeadZoneComboBox->addItem(QString::number(temp), temp);
    }
}

void GameControllerMappingDialog::changeAxisDeadZone(int index)
{
    unsigned int value = ui->axisDeadZoneComboBox->itemData(index).toInt();
    if (value >= 5000 && value <= 32000)
    {
        device->getActiveSetJoystick()->raiseAxesDeadZones(value);
    }
}

void GameControllerMappingDialog::updateLastAxisLineEdit(int value)
{
    if (abs(value) >= 5000)
    {
        JoyAxis *tempAxis = static_cast<JoyAxis*>(sender());
        QString temp;
        if (device->isGameController())
        {
            GameController *controller = static_cast<GameController*>(device);
            temp = QString("%1: %2").arg(controller->getBindStringForAxis(tempAxis->getIndex(), false))
                                    .arg(value);
        }
        else
        {
            temp = QString("Axis %1: %2").arg(tempAxis->getRealJoyIndex())
                                         .arg(value);
        }

        ui->lastAxisEventLineEdit->setText(temp);
    }
}
