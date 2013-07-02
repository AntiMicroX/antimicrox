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
    void loadConfigFile(QString fileLocation);

protected:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *configHorizontalLayout;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *saveAsButton;
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

    Joystick *joystick;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(Joystick *joystick);
    void joystickConfigChanged(int index);
    void joystickAxisRefreshLabels(int axisIndex);

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
    void showStickDialog();
    void showStickAssignmentDialog();
    void showDPadDialog();
    void showQuickSetDialog();

    void changeSetOne();
    void changeSetTwo();
    void changeSetThree();
    void changeSetFour();
    void changeSetFive();
    void changeSetSix();
    void changeSetSeven();
    void changeSetEight();
    void changeCurrentSet(int index);
    void openStickButtonDialog();
};

#endif // JOYTABWIDGET_H
