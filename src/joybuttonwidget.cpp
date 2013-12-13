#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "joybuttonwidget.h"

JoyButtonWidget::JoyButtonWidget(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{
    this->button = button;

    isflashing = false;
    displayNames = false;
    leftAlignText = false;

    refreshLabel();

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(refreshLabel()));
}

JoyButton* JoyButtonWidget::getJoyButton()
{
    return button;
}

void JoyButtonWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::refreshLabel()
{
    setText(generateLabel());
}

void JoyButtonWidget::disableFlashes()
{
    disconnect (button, SIGNAL(clicked(int)), this, 0);
    disconnect (button, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    connect (button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect (button, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool JoyButtonWidget::isButtonFlashing()
{
    return isflashing;
}

QString JoyButtonWidget::generateLabel()
{
    QString temp;
    temp = button->getName(false, displayNames).replace("&", "&&");
    return temp;
}

void JoyButtonWidget::toggleNameDisplay()
{
    displayNames = !displayNames;
    refreshLabel();
}

void JoyButtonWidget::setDisplayNames(bool display)
{
    displayNames = display;
}

bool JoyButtonWidget::isDisplayingNames()
{
    return displayNames;
}

void JoyButtonWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QFont tempScaledFont = painter.font();
    tempScaledFont.setPointSize(10);
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
