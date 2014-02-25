#include <QSettings>
#include <common.h>
#include <QDir>
#include <QFileDialog>

#include "mainsettingsdialog.h"
#include "ui_mainsettingsdialog.h"

MainSettingsDialog::MainSettingsDialog(QSettings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->settings = settings;
    //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);

    fillControllerMappingsTable();
    QString defaultProfileDir = settings->value("DefaultProfileDir", "").toString();
    int numberRecentProfiles = settings->value("NumberRecentProfiles", 5).toInt();

    if (!defaultProfileDir.isEmpty() && QDir(defaultProfileDir).exists())
    {
        ui->profileDefaultDirLineEdit->setText(defaultProfileDir);
    }

    ui->numberRecentProfileSpinBox->setValue(numberRecentProfiles);

    connect(ui->categoriesListWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
    connect(ui->controllerMappingsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(mappingsTableItemChanged(QTableWidgetItem*)));
    connect(ui->mappingDeletePushButton, SIGNAL(clicked()), this, SLOT(deleteMappingRow()));
    connect(ui->mappngInsertPushButton, SIGNAL(clicked()), this, SLOT(insertMappingRow()));
    //connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(syncMappingSettings()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveNewSettings()));
    connect(ui->profileOpenDirPushButton, SIGNAL(clicked()), this, SLOT(selectDefaultProfileDir()));
}

MainSettingsDialog::~MainSettingsDialog()
{
    delete ui;
}

void MainSettingsDialog::fillControllerMappingsTable()
{
    /*QList<QVariant> tempvariant = bindingValues(bind);
    QTableWidgetItem* item = new QTableWidgetItem();
    ui->buttonMappingTableWidget->setItem(associatedRow, 0, item);
    item->setText(temptext);
    item->setData(Qt::UserRole, tempvariant);
    */

    QHash<QString, QList<QVariant> > tempHash;

    settings->beginGroup("Mappings");

    QStringList mappings = settings->allKeys();
    QStringListIterator iter(mappings);
    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString tempGUID;

        if (tempkey.contains("Disable"))
        {
            bool disableGameController = settings->value(tempkey, false).toBool();
            tempGUID = tempkey.remove("Disable");
            insertTempControllerMapping(tempHash, tempGUID);
            if (tempHash.contains(tempGUID))
            {
                QList<QVariant> templist = tempHash.value(tempGUID);
                templist.replace(2, QVariant(disableGameController));
                tempHash.insert(tempGUID, templist); // Overwrite original list
            }
        }
        else
        {
            QString mappingString = settings->value(tempkey, QString()).toString();
            if (!mappingString.isEmpty())
            {
                tempGUID = tempkey;
                insertTempControllerMapping(tempHash, tempGUID);
                if (tempHash.contains(tempGUID))
                {
                    QList<QVariant> templist = tempHash.value(tempGUID);
                    templist.replace(1, mappingString);
                    tempHash.insert(tempGUID, templist); // Overwrite original list
                }
            }
        }
    }

    settings->endGroup();

    QHashIterator<QString, QList<QVariant> > iter2(tempHash);
    int i = 0;
    while (iter2.hasNext())
    {
        ui->controllerMappingsTableWidget->insertRow(i);

        QList<QVariant> templist = iter2.next().value();
        QTableWidgetItem* item = new QTableWidgetItem(templist.at(0).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, iter2.key());
        ui->controllerMappingsTableWidget->setItem(i, 0, item);

        item = new QTableWidgetItem(templist.at(1).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, iter2.key());
        ui->controllerMappingsTableWidget->setItem(i, 1, item);

        bool disableController = templist.at(2).toBool();
        item = new QTableWidgetItem();
        item->setCheckState(disableController ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, iter2.key());
        ui->controllerMappingsTableWidget->setItem(i, 2, item);

        i++;
    }
}

void MainSettingsDialog::insertTempControllerMapping(QHash<QString, QList<QVariant> > &hash, QString newGUID)
{
    if (!newGUID.isEmpty() && !hash.contains(newGUID))
    {
        QList<QVariant> templist;
        templist.append(QVariant(newGUID));
        templist.append(QVariant(""));
        templist.append(QVariant(false));

        hash.insert(newGUID, templist);
    }
}

void MainSettingsDialog::mappingsTableItemChanged(QTableWidgetItem *item)
{
    int column = item->column();
    int row = item->row();

    if (column == 0 && !item->text().isEmpty())
    {
        QTableWidgetItem *disableitem = ui->controllerMappingsTableWidget->item(row, column);
        if (disableitem)
        {
            disableitem->setData(Qt::UserRole, item->text());
        }

        item->setData(Qt::UserRole, item->text());
    }
}

void MainSettingsDialog::insertMappingRow()
{
    int insertRowIndex = ui->controllerMappingsTableWidget->rowCount();
    ui->controllerMappingsTableWidget->insertRow(insertRowIndex);

    QTableWidgetItem* item = new QTableWidgetItem();
    //item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    //item->setData(Qt::UserRole, iter2.key());
    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 0, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    //item->setData(Qt::UserRole, iter2.key());
    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 1, item);

    item = new QTableWidgetItem();
    item->setCheckState(Qt::Unchecked);
    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 2, item);
}

void MainSettingsDialog::deleteMappingRow()
{
    int row = ui->controllerMappingsTableWidget->currentRow();

    if (row >= 0)
    {
        ui->controllerMappingsTableWidget->removeRow(row);
    }
}

void MainSettingsDialog::syncMappingSettings()
{
    settings->beginGroup("Mappings");
    settings->remove("");

    for (int i=0; i < ui->controllerMappingsTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *itemGUID = ui->controllerMappingsTableWidget->item(i, 0);
        QTableWidgetItem *itemMapping = ui->controllerMappingsTableWidget->item(i, 1);
        QTableWidgetItem *itemDisable = ui->controllerMappingsTableWidget->item(i, 2);

        if (itemGUID && !itemGUID->text().isEmpty() && itemDisable)
        {
            bool disableController = itemDisable->checkState() == Qt::Checked ? true : false;
            if (itemMapping && !itemMapping->text().isEmpty())
            {
                settings->setValue(itemGUID->text(), itemMapping->text());
            }

            settings->setValue(QString("%1Disable").arg(itemGUID->text()), disableController);
        }
    }

    settings->endGroup();
}

void MainSettingsDialog::saveNewSettings()
{
    //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    syncMappingSettings();
    QString oldProfileDir = settings->value("DefaultProfileDir", "").toString();
    QString possibleProfileDir = ui->profileDefaultDirLineEdit->text();

    if (oldProfileDir != possibleProfileDir)
    {
        if (QFileInfo(possibleProfileDir).exists())
        {
            settings->setValue("DefaultProfileDir", possibleProfileDir);
        }
        else if (possibleProfileDir.isEmpty())
        {
            settings->remove("DefaultProfileDir");
        }
    }

    int numRecentProfiles = ui->numberRecentProfileSpinBox->value();
    settings->setValue("NumberRecentProfiles", numRecentProfiles);
    settings->sync();
}

void MainSettingsDialog::selectDefaultProfileDir()
{
    QString lookupDir = QDir::homePath();
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Default Profile Directory"), lookupDir);
    if (!directory.isEmpty() && QFileInfo(directory).exists())
    {
        ui->profileDefaultDirLineEdit->setText(directory);
    }
}
