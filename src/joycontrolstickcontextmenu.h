#ifndef JOYCONTROLSTICKCONTEXTMENU_H
#define JOYCONTROLSTICKCONTEXTMENU_H

#include <QMenu>

#include "joycontrolstick.h"

class JoyControlStickContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit JoyControlStickContextMenu(JoyControlStick *stick, QWidget *parent = 0);
    void buildMenu();

protected:
    int getPresetIndex();

    JoyControlStick *stick;

signals:

public slots:

private slots:
    void setStickPreset();
    void setStickMode();
    void openMouseSettingsDialog();
};

#endif // JOYCONTROLSTICKCONTEXTMENU_H
