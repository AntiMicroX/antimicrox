#include <QDebug>
#include <QList>
#include <QListIterator>
#include <QTableWidgetItem>
#include <QAbstractItemModel>
#include <QModelIndexList>
#include <QVariant>
#include <QStringList>
#include <QMessageBox>

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

GameControllerMappingDialog::GameControllerMappingDialog(InputDevice *device, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameControllerMappingDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->device = device;

    device->getActiveSetJoystick()->setIgnoreEventState(true);
    device->getActiveSetJoystick()->release();

    if (qobject_cast<GameController*>(device) != 0)
    {
        populateGameControllerBindings(static_cast<GameController*>(device));
    }

    QString tempWindowTitle = QString(tr("Game Controller Mapping (%1)")).arg(device->getSDLName());
    setWindowTitle(tempWindowTitle);

    enableDeviceConnections();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(testsave()));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(testOther(QAbstractButton*)));
    connect(this, SIGNAL(finished(int)), this, SLOT(enableButtonEvents()));
}

GameControllerMappingDialog::~GameControllerMappingDialog()
{
    delete ui;
}

void GameControllerMappingDialog::testButtonAssign(int buttonindex)
{
    QTableWidgetItem* item = ui->buttonMappingTableWidget->currentItem();
    int column = ui->buttonMappingTableWidget->currentColumn();
    int row = ui->buttonMappingTableWidget->currentRow();

    if (row < 17)
    {
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
    }
}

void GameControllerMappingDialog::testAxisAssign(int axis, int value)
{
    Q_UNUSED(value);

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
    }
}

void GameControllerMappingDialog::testDPadAssign(int dpad, int buttonindex)
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
    }
}

void GameControllerMappingDialog::testsave()
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

    qDebug() << "THIS IS IT";
    qDebug() << templist.join(",");
    qDebug();

    QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    settings.setValue(QString("Mappings/").append(device->getGUIDString()), templist.join(","));
    emit mappingUpdate(templist.join(","), device);
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

void GameControllerMappingDialog::testOther(QAbstractButton *button)
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
            connect(device, SIGNAL(rawButtonClick(int)), this, SLOT(testButtonAssign(int)));
            connect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(testAxisAssign(int,int)));
            connect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(testDPadAssign(int,int)));
        }
    }
}

void GameControllerMappingDialog::removeControllerMapping()
{
    QSettings settings(PadderCommon::configFileName, QSettings::IniFormat);
    settings.remove(QString("Mappings/").append(device->getGUIDString()));
}

void GameControllerMappingDialog::enableDeviceConnections()
{
    connect(device, SIGNAL(rawButtonClick(int)), this, SLOT(testButtonAssign(int)));
    connect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(testAxisAssign(int,int)));
    connect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(testDPadAssign(int,int)));
}

void GameControllerMappingDialog::disableDeviceConnections()
{
    disconnect(device, SIGNAL(rawButtonClick(int)), this, SLOT(testButtonAssign(int)));
    disconnect(device, SIGNAL(rawAxisActivated(int,int)), this, SLOT(testAxisAssign(int,int)));
    disconnect(device, SIGNAL(rawDPadButtonClick(int,int)), this, SLOT(testDPadAssign(int,int)));
}

void GameControllerMappingDialog::enableButtonEvents()
{
    device->getActiveSetJoystick()->setIgnoreEventState(false);
    device->getActiveSetJoystick()->release();
}
