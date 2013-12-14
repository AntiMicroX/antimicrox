#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "flashbuttonwidget.h"

FlashButtonWidget::FlashButtonWidget(QWidget *parent) :
    QPushButton(parent)
{
    isflashing = false;
    displayNames = false;
    leftAlignText = false;
}

void FlashButtonWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void FlashButtonWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void FlashButtonWidget::refreshLabel()
{
    setText(generateLabel());
}

bool FlashButtonWidget::isButtonFlashing()
{
    return isflashing;
}

void FlashButtonWidget::toggleNameDisplay()
{
    displayNames = !displayNames;
    refreshLabel();
}

void FlashButtonWidget::setDisplayNames(bool display)
{
    displayNames = display;
}

bool FlashButtonWidget::isDisplayingNames()
{
    return displayNames;
}

void FlashButtonWidget::paintEvent(QPaintEvent *event)
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
