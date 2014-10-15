#ifndef DPADCONTEXTMENU_H
#define DPADCONTEXTMENU_H

#include <QMenu>

#include "joydpad.h"

class DPadContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit DPadContextMenu(JoyDPad *dpad, QWidget *parent = 0);
    void buildMenu();

protected:
    int getPresetIndex();

    JoyDPad *dpad;

signals:

public slots:

private slots:
    void setDPadPreset();
    void setDPadMode();
    void openMouseSettingsDialog();
};

#endif // DPADCONTEXTMENU_H
