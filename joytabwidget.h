#ifndef JOYTABWIDGET_H
#define JOYTABWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpacerItem>
#include <QFileDialog>
#include <QSettings>
#include <QHash>

#include "joystick.h"
#include "axiseditdialog.h"
#include "buttoneditdialog.h"

class JoyTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoyTabWidget(Joystick *joystick, QWidget *parent = 0);
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    QHash<int, QString>* recentConfigs();
    void setCurrentConfig(int index);
    int getCurrentConfigIndex();
    QString getCurrentConfigName();

protected:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *configHorizontalLayout;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *saveAsButton;
    QComboBox *configBox;
    QGridLayout *gridLayout;
    QSpacerItem *spacer1;
    QSpacerItem *spacer2;
    QSpacerItem *spacer3;
    QFileDialog *fileDialog;
    ButtonEditDialog *buttonDialog;
    AxisEditDialog *axisDialog;

    Joystick *joystick;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(Joystick *joystick);
    void joystickConfigChanged(int index);

public slots:
    void openConfigFileDialog();
    void fillButtons();

private slots:
    void saveConfigFile();
    void resetJoystick();
    void saveAsConfig();
    void changeJoyConfig(int index);
    void showAxisDialog();
    void showButtonDialog();
};

#endif // JOYTABWIDGET_H
