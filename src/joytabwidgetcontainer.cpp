#include "joytabwidgetcontainer.h"

JoyTabWidgetContainer::JoyTabWidgetContainer(QWidget *parent) :
    QTabWidget(parent)
{
}

int JoyTabWidgetContainer::addTab(QWidget *widget, const QString &string)
{
    return QTabWidget::addTab(widget, string);
}

int JoyTabWidgetContainer::addTab(JoyTabWidget *widget, const QString &string)
{
    InputDevice *joystick = widget->getJoystick();

    if (joystick)
    {
        enableFlashes(joystick);
    }

    return QTabWidget::addTab(widget, string);
}

void JoyTabWidgetContainer::flash()
{
    InputDevice *joystick = static_cast<InputDevice*>(sender());

    bool found = false;
    for (int i = 0; i < tabBar()->count() && !found; i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(widget(i));
        if (tab && tab->getJoystick() == joystick)
        {
            tabBar()->setTabTextColor(i, Qt::red);
            found = true;
        }
    }
}

void JoyTabWidgetContainer::unflash()
{
    InputDevice *joystick = static_cast<InputDevice*>(sender());

    bool found = false;
    for (int i = 0; i < tabBar()->count() && !found; i++)
    {
        JoyTabWidget *tab = static_cast<JoyTabWidget*>(widget(i));
        if (tab && tab->getJoystick() == joystick)
        {
            tabBar()->setTabTextColor(i, Qt::black);
            found = true;
        }
    }
}

void JoyTabWidgetContainer::disableFlashes(InputDevice *joystick)
{
    unflash();

    disconnect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(joystick, SIGNAL(released(int)), this, SLOT(unflash()));
}

void JoyTabWidgetContainer::enableFlashes(InputDevice *joystick)
{
    connect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(joystick, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}
