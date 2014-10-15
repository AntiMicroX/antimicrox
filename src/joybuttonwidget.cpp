#include <QMenu>

#include "joybuttonwidget.h"
#include "joybuttoncontextmenu.h"

JoyButtonWidget::JoyButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->button = button;

    refreshLabel();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    //connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    //connect(button, SIGNAL(buttonNameChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(propertyUpdated()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()), Qt::QueuedConnection);
}

JoyButton* JoyButtonWidget::getJoyButton()
{
    return button;
}

void JoyButtonWidget::disableFlashes()
{
    disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

QString JoyButtonWidget::generateLabel()
{
    QString temp;
    temp = button->getName(false, displayNames).replace("&", "&&");
    return temp;
}

void JoyButtonWidget::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    JoyButtonContextMenu *contextMenu = new JoyButtonContextMenu(button, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}
