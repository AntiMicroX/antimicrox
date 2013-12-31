#ifndef JOYTABWIDGETCONTAINER_H
#define JOYTABWIDGETCONTAINER_H

#include <QTabWidget>

#include "joystick.h"
#include "joytabwidget.h"

class JoyTabWidgetContainer : public QTabWidget
{
    Q_OBJECT
public:
    explicit JoyTabWidgetContainer(QWidget *parent = 0);

    int addTab(QWidget *widget, const QString &string);
    int addTab(JoyTabWidget *widget, const QString &string);

protected:

signals:

public slots:
    void disableFlashes(InputDevice *joystick);
    void enableFlashes(InputDevice *joystick);

private slots:
    void flash();
    void unflash();
};

#endif // JOYTABWIDGETCONTAINER_H
