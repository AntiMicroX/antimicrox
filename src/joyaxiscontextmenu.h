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

protected:
    int getPresetIndex();

    JoyAxis *axis;

signals:

public slots:

private slots:
    void setAxisPreset();
    void openMouseSettingsDialog();
};

#endif // JOYAXISCONTEXTMENU_H
