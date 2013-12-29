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
    tabBar()->setTabTextColor(joystick->getJoyNumber(), Qt::red);
}

void JoyTabWidgetContainer::unflash()
{
    InputDevice *joystick = static_cast<InputDevice*>(sender());
    tabBar()->setTabTextColor(joystick->getJoyNumber(), Qt::black);
}

void JoyTabWidgetContainer::disableFlashes(InputDevice *joystick)
{
    disconnect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(joystick, SIGNAL(released(int)), this, SLOT(unflash()));
}

void JoyTabWidgetContainer::enableFlashes(InputDevice *joystick)
{
    connect(joystick, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(joystick, SIGNAL(released(int)), this, SLOT(unflash()));
}
