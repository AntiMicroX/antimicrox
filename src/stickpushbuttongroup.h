#ifndef STICKPUSHBUTTONGROUP_H
#define STICKPUSHBUTTONGROUP_H

#include <QGridLayout>

#include "joycontrolstick.h"
#include "joycontrolstickpushbutton.h"
#include "joycontrolstickbuttonpushbutton.h"

class StickPushButtonGroup : public QGridLayout
{
    Q_OBJECT
public:
    explicit StickPushButtonGroup(JoyControlStick *stick, bool displayNames = false, QWidget *parent = 0);
    JoyControlStick *getStick();

protected:
    void generateButtons();

    JoyControlStick *stick;
    bool displayNames;

    JoyControlStickButtonPushButton *upButton;
    JoyControlStickButtonPushButton *downButton;
    JoyControlStickButtonPushButton *leftButton;
    JoyControlStickButtonPushButton *rightButton;

    JoyControlStickButtonPushButton *upLeftButton;
    JoyControlStickButtonPushButton *upRightButton;
    JoyControlStickButtonPushButton *downLeftButton;
    JoyControlStickButtonPushButton *downRightButton;

    JoyControlStickPushButton *stickWidget;

signals:
    void buttonSlotChanged();

public slots:
    void changeButtonLayout();
    void toggleNameDisplay();

private slots:
    void propogateSlotsChanged();
    void openStickButtonDialog();
    void showStickDialog();
};

#endif // STICKPUSHBUTTONGROUP_H
