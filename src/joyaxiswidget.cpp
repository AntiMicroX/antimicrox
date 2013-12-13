#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, QWidget *parent) :
    QPushButton(parent)
{
    this->axis = axis;

    isflashing = false;
    displayNames = false;
    leftAlignText = false;

    setText(generateLabel());

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(axis, SIGNAL(throttleChanged()), this, SLOT(refreshLabel()));
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::refreshLabel()
{
    setText(generateLabel());
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), this, 0);
    disconnect(axis, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool JoyAxisWidget::isButtonFlashing()
{
    return isflashing;
}

QString JoyAxisWidget::generateLabel()
{
    QString temp;
    temp = axis->getName(false, displayNames).replace("&", "&&");
    return temp;
}

void JoyAxisWidget::toggleNameDisplay()
{
    displayNames = !displayNames;
    refreshLabel();
}

void JoyAxisWidget::setDisplayNames(bool display)
{
    displayNames = display;
}

bool JoyAxisWidget::isDisplayingNames()
{
    return displayNames;
}

void JoyAxisWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QFont tempScaledFont = painter.font();
    QFont temp;
    tempScaledFont.setPointSize(temp.pointSize());
    QFontMetrics fm(tempScaledFont);

    bool reduce = false;
    while ((this->width() < fm.width(text())) && tempScaledFont.pointSize() >= 7)
    {
        tempScaledFont.setPointSize(tempScaledFont.pointSize()-1);
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
