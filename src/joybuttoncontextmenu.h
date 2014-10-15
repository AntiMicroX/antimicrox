#ifndef JOYBUTTONCONTEXTMENU_H
#define JOYBUTTONCONTEXTMENU_H

#include <QMenu>
#include "joybutton.h"

class JoyButtonContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit JoyButtonContextMenu(JoyButton *button, QWidget *parent = 0);
    void buildMenu();

protected:
    JoyButton *button;

signals:

private slots:
    void switchToggle();
    void switchTurbo();
    void switchSetMode();
    void disableSetMode();
};

#endif // JOYBUTTONCONTEXTMENU_H
