#ifndef VIRTUALDPADPUSHBUTTON_H
#define VIRTUALDPADPUSHBUTTON_H

#include <QPushButton>
#include <QPaintEvent>

#include "vdpad.h"

class VirtualDPadPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit VirtualDPadPushButton(VDPad *vdpad, QWidget *parent = 0);
    VDPad* getVDPad();
    bool isButtonFlashing();
    void setDisplayNames(bool display);
    bool isDisplayingNames();

protected:
    virtual void paintEvent(QPaintEvent *event);
    QString generateLabel();

    VDPad *vdpad;
    bool isflashing;
    bool displayNames;
    bool leftAlignText;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void disableFlashes();
    void enableFlashes();
    void toggleNameDisplay();

private slots:
    void flash();
    void unflash();
};

#endif // VIRTUALDPADPUSHBUTTON_H
