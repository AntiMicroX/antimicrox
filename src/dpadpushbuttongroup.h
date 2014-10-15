#ifndef DPADPUSHBUTTONGROUP_H
#define DPADPUSHBUTTONGROUP_H

#include <QGridLayout>

#include "joydpad.h"
#include "joydpadbuttonwidget.h"
#include "dpadpushbutton.h"

class DPadPushButtonGroup : public QGridLayout
{
    Q_OBJECT
public:
    explicit DPadPushButtonGroup(JoyDPad *dpad, bool displayNames = false, QWidget *parent = 0);
    JoyDPad *getDPad();

protected:
    void generateButtons();

    JoyDPad *dpad;
    bool displayNames;

    JoyDPadButtonWidget *upButton;
    JoyDPadButtonWidget *downButton;
    JoyDPadButtonWidget *leftButton;
    JoyDPadButtonWidget *rightButton;

    JoyDPadButtonWidget *upLeftButton;
    JoyDPadButtonWidget *upRightButton;
    JoyDPadButtonWidget *downLeftButton;
    JoyDPadButtonWidget *downRightButton;

    DPadPushButton *dpadWidget;

signals:
    void buttonSlotChanged();

public slots:
    void changeButtonLayout();
    void toggleNameDisplay();

private slots:
    void propogateSlotsChanged();
    void openDPadButtonDialog();
    void showDPadDialog();
};

#endif // DPADPUSHBUTTONGROUP_H
