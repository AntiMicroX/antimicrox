#ifndef MAINSETTINGSDIALOG_H
#define MAINSETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QTableWidgetItem>

namespace Ui {
class MainSettingsDialog;
}

class MainSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettingsDialog(QSettings *settings, QWidget *parent = 0);
    ~MainSettingsDialog();

protected:
    void fillControllerMappingsTable();

    QSettings *settings;

private:
    void insertTempControllerMapping(QHash<QString, QList<QVariant> > &hash, QString newGUID);

    Ui::MainSettingsDialog *ui;

protected slots:
    void mappingsTableItemChanged(QTableWidgetItem *item);
    void insertMappingRow();
    void deleteMappingRow();
    void syncMappingSettings();
    void saveNewSettings();
    void selectDefaultProfileDir();
};

#endif // MAINSETTINGSDIALOG_H
