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
#include <QStackedWidget>
#include <QScrollArea>
#include <QIcon>

#include "joystick.h"
#include "axiseditdialog.h"
#include "inputdevice.h"


class JoyTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoyTabWidget(InputDevice *joystick, QSettings *settings, QWidget *parent = 0);

    void saveSettings();
    void loadSettings(bool forceRefresh=false);
    QHash<int, QString>* recentConfigs();
    void setCurrentConfig(int index);
    int getCurrentConfigIndex();
    QString getCurrentConfigName();
    void loadConfigFile(QString fileLocation);
    InputDevice *getJoystick();
    void unloadConfig();
    bool isDisplayingNames();

protected:
    void removeCurrentButtons();
    QString preferredProfileDir(QSettings *settings);

    QVBoxLayout *verticalLayout;
    QHBoxLayout *configHorizontalLayout;
    QPushButton *removeButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *namesPushButton;
    QPushButton *saveAsButton;
    QPushButton *delayButton;
    QComboBox *configBox;
    QGridLayout *gridLayout;
    QGridLayout *gridLayout2;
    QGridLayout *gridLayout3;
    QGridLayout *gridLayout4;
    QGridLayout *gridLayout5;
    QGridLayout *gridLayout6;
    QGridLayout *gridLayout7;
    QGridLayout *gridLayout8;

    QSpacerItem *spacer1;
    QSpacerItem *spacer2;
    QSpacerItem *spacer3;
    AxisEditDialog *axisDialog;

    QPushButton *setPushButton1;
    QPushButton *setPushButton2;
    QPushButton *setPushButton3;
    QPushButton *setPushButton4;
    QPushButton *setPushButton5;
    QPushButton *setPushButton6;
    QPushButton *setPushButton7;
    QPushButton *setPushButton8;

    QPushButton *setsMenuButton;
    QAction *setAction1;
    QAction *setAction2;
    QAction *setAction3;
    QAction *setAction4;
    QAction *setAction5;
    QAction *setAction6;
    QAction *setAction7;
    QAction *setAction8;

    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *stickAssignPushButton;
    QPushButton *quickSetPushButton;
    QSpacerItem *verticalSpacer_2;
    QStackedWidget *stackedWidget_2;
    QWidget *page;
    QWidget *page_2;
    QWidget *page_3;
    QWidget *page_4;
    QWidget *page_5;
    QWidget *page_6;
    QWidget *page_7;
    QWidget *page_8;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer_3;

    InputDevice *joystick;
    bool displayingNames;
    QSettings *settings;

    static const int DEFAULTNUMBERPROFILES = 5;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(InputDevice *joystick);
    void joystickConfigChanged(int index);
    void joystickAxisRefreshLabels(int axisIndex);
    void namesDisplayChanged(bool status);

public slots:
    void openConfigFileDialog();
    void fillButtons();
    void saveDeviceSettings(bool sync=false);
    void loadDeviceSettings();
    void changeNameDisplay(bool displayNames);
    void changeCurrentSet(int index);
    void refreshSetButtons();

private slots:
    void saveConfigFile();
    void resetJoystick();
    void saveAsConfig();
    void removeConfig();
    void changeJoyConfig(int index);
    void showAxisDialog();
    void showButtonDialog();
    void showStickDialog();
    void showStickAssignmentDialog();
    void showDPadDialog();
    void showQuickSetDialog();
    void showKeyDelayDialog();
    void showSetNamesDialog();
    void toggleNames();

    void changeSetOne();
    void changeSetTwo();
    void changeSetThree();
    void changeSetFour();
    void changeSetFive();
    void changeSetSix();
    void changeSetSeven();
    void changeSetEight();
    void openStickButtonDialog();
};

#endif // JOYTABWIDGET_H
