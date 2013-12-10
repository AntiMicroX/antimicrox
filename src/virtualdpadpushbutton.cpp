#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "virtualdpadpushbutton.h"

VirtualDPadPushButton::VirtualDPadPushButton(VDPad *vdpad, QWidget *parent) :
    QPushButton(parent)
{
    this->vdpad = vdpad;

    isflashing = false;
    displayNames = false;
    leftAlignText = false;

    refreshLabel();

    connect(vdpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(vdpad, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

VDPad* VirtualDPadPushButton::getVDPad()
{
    return vdpad;
}

void VirtualDPadPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void VirtualDPadPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void VirtualDPadPushButton::refreshLabel()
{
    setText(generateLabel());
}

QString VirtualDPadPushButton::generateLabel()
{
    QString temp;

    if (!vdpad->getDpadName().isEmpty())
    {
        temp.append(vdpad->getName(false, displayNames));
    }
    else
    {
        temp.append(vdpad->getName(true));
    }

    return temp;
}

void VirtualDPadPushButton::disableFlashes()
{
    disconnect(vdpad, SIGNAL(active(int)), this, 0);
    disconnect(vdpad, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void VirtualDPadPushButton::enableFlashes()
{
    connect(vdpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(vdpad, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool VirtualDPadPushButton::isButtonFlashing()
{
    return isflashing;
}

void VirtualDPadPushButton::toggleNameDisplay()
{
    displayNames = !displayNames;
    refreshLabel();
}

void VirtualDPadPushButton::setDisplayNames(bool display)
{
    displayNames = display;
}

bool VirtualDPadPushButton::isDisplayingNames()
{
    return displayNames;
}

void VirtualDPadPushButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QFont tempScaledFont = painter.font();
    tempScaledFont.setPointSize(8);
    QFontMetrics fm(tempScaledFont);

    bool reduce = false;
    while ((this->width() < fm.width(text())) && tempScaledFont.pointSize() >= 7)
    {
        tempScaledFont.setPointSize(painter.font().pointSize()-1);
        painter.setFont(tempScaledFont);
        fm = painter.fontMetrics();
        reduce = true;
    }

    bool changeFontSize = this->font().pointSize() != tempScaledFont.pointSize();
    if (changeFontSize)
    {
        if (reduce && !leftAlignText)
        {
            leftAlignText = !leftAlignText;
            setStyleSheet("text-align: left;");
            this->style()->unpolish(this);
            this->style()->polish(this);
        }
        else if (!reduce && leftAlignText)
        {
            leftAlignText = !leftAlignText;
            setStyleSheet("text-align: center;");
            this->style()->unpolish(this);
            this->style()->polish(this);
        }

        this->setFont(tempScaledFont);
    }

    QPushButton::paintEvent(event);
}
