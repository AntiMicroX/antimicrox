#ifndef JOYCONTROLSTICKBUTTONPUSHBUTTON_H
#define JOYCONTROLSTICKBUTTONPUSHBUTTON_H

#include <QPushButton>

#include "joycontrolstickbutton.h"

class JoyControlStickButtonPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit JoyControlStickButtonPushButton(JoyControlStickButton *button, QWidget *parent = 0);
    explicit JoyControlStickButtonPushButton(QWidget *parent = 0);

    JoyControlStickButton* getButton();
    bool isButtonFlashing();
    void setButton(JoyControlStickButton *button);
    void setDisplayNames(bool display);
    bool isDisplayingNames();

protected:
    JoyControlStickButton *button;
    bool isflashing;
    bool displayNames;
    
signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void unflash();
    void disableFlashes();
    void enableFlashes();
    void toggleNameDisplay();

private slots:
    void flash();
};

#endif // JOYCONTROLSTICKBUTTONPUSHBUTTON_H
