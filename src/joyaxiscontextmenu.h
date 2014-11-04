#ifndef JOYAXISCONTEXTMENU_H
#define JOYAXISCONTEXTMENU_H

#include <QMenu>

#include "joyaxis.h"

class JoyAxisContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit JoyAxisContextMenu(JoyAxis *axis, QWidget *parent = 0);
    void buildMenu();
    void buildAxisMenu();
    void buildTriggerMenu();

protected:
    int getPresetIndex();
    int getTriggerPresetIndex();

    JoyAxis *axis;

signals:

public slots:

private slots:
    void setAxisPreset();
    void setTriggerPreset();
    void openMouseSettingsDialog();
};

#endif // JOYAXISCONTEXTMENU_H
