#include "dpadpushbutton.h"
#include "dpadcontextmenu.h"

DPadPushButton::DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->dpad = dpad;

    refreshLabel();
    enableFlashes();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

JoyDPad* DPadPushButton::getDPad()
{
    return dpad;
}

QString DPadPushButton::generateLabel()
{
    QString temp;
    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName(false, displayNames));
    }
    else
    {
        temp.append(dpad->getName());
    }

    return temp;
}

void DPadPushButton::disableFlashes()
{
    disconnect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    connect(dpad, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(dpad, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

void DPadPushButton::showContextMenu(const QPoint &point)
{
    QPoint globalPos = this->mapToGlobal(point);
    DPadContextMenu *contextMenu = new DPadContextMenu(dpad, this);
    contextMenu->buildMenu();
    contextMenu->popup(globalPos);
}
