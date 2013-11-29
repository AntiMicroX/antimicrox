#ifndef MOUSESETTINGSDIALOG_H
#define MOUSESETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class MouseSettingsDialog;
}

class MouseSettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MouseSettingsDialog(QWidget *parent = 0);
    ~MouseSettingsDialog();
    
protected:
    Ui::MouseSettingsDialog *ui;

public slots:
    void changeSensitivityStatus(int index);
    void changeSpringSpinBoxStatus(int index);
    void changeMouseSpeedBoxStatus(int index);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    void changeSmoothingStatus(int index);
    void updateWheelSpeedLabel(int value);
};

#endif // MOUSESETTINGSDIALOG_H
