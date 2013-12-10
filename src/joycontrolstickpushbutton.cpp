#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "joycontrolstickpushbutton.h"

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, QWidget *parent) :
    QPushButton(parent)
{
    this->stick = stick;

    isflashing = false;
    displayNames = false;
    leftAlignText = false;

    refreshLabel();

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(refreshLabel()));
}

JoyControlStick* JoyControlStickPushButton::getStick()
{
    return stick;
}

void JoyControlStickPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickPushButton::refreshLabel()
{
    setText(generateLabel());
}

QString JoyControlStickPushButton::generateLabel()
{
    QString temp;
    if (!stick->getStickName().isEmpty() && displayNames)
    {
        temp.append(stick->getStickName());
    }
    else
    {
        temp.append(tr("Stick")).append(" ").append(QString::number(stick->getRealJoyIndex()));
    }

    return temp;
}

void JoyControlStickPushButton::disableFlashes()
{
    disconnect(stick, SIGNAL(active(int, int)), this, 0);
    disconnect(stick, SIGNAL(released(int, int)), this, 0);
    this->unflash();
}

void JoyControlStickPushButton::enableFlashes()
{
    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
}

bool JoyControlStickPushButton::isButtonFlashing()
{
    return isflashing;
}

void JoyControlStickPushButton::toggleNameDisplay()
{
    displayNames = !displayNames;
    refreshLabel();
}

void JoyControlStickPushButton::setDisplayNames(bool display)
{
    displayNames = display;
}

bool JoyControlStickPushButton::isDisplayingNames()
{
    return displayNames;
}

void JoyControlStickPushButton::paintEvent(QPaintEvent *event)
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
