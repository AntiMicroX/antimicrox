#ifndef JOYCONTROLSTICKEDITDIALOG_H
#define JOYCONTROLSTICKEDITDIALOG_H

#include <QDialog>

#include "joycontrolstick.h"

namespace Ui {
class JoyControlStickEditDialog;
}

class JoyControlStickEditDialog : public QDialog
{
    Q_OBJECT
    
public:
    //explicit JoyControlStickEditDialog(QWidget *parent = 0);
    explicit JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent = 0);
    ~JoyControlStickEditDialog();
    
protected:
    JoyControlStick *stick;

private:
    Ui::JoyControlStickEditDialog *ui;

private slots:
    void implementPresets(int index);
    void openAdvancedUpDialog();
    void openAdvancedDownDialog();
    void openAdvancedLeftDialog();
    void openAdvancedRightDialog();

    void refreshUpButtonLabel();
    void refreshDownButtonLabel();
    void refreshLeftButtonLabel();
    void refreshRightButtonLabel();

    void refreshStickStats(int x, int y);
    void syncSpeedSpinBoxes();
    void changeMouseSpeedsInterface(bool value);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);

    void checkMaxZone(int value);
};

#endif // JOYCONTROLSTICKEDITDIALOG_H
