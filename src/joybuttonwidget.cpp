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

    refreshLabel();

    connect (button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect (button, SIGNAL(released(int)), this, SLOT(unflash()));
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
    disconnect (button, SIGNAL(clicked(int)), 0, 0);
    disconnect (button, SIGNAL(released(int)), 0, 0);
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
    temp = button->getName().replace("&", "&&");
    return temp;
}

void JoyButtonWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QFontMetrics fm = this->fontMetrics();
    //QString temp = stick->getName();
    QFont tempWidgetFont = this->font();
    //QFontMetrics fm(this->font());
    //QString temp = fm.elidedText(stick->getName(), Qt::ElideRight, this->width());
    //this->setText(temp);
    //qDebug() << "FM WIDTH B4: " << fm.width(stick->getName()) << " " << text();
    QFont tempScaledFont = painter.font();

    while ((this->width() < fm.width(text())) && tempScaledFont.pointSize() >= 6)
    {
        tempScaledFont.setPointSize(painter.font().pointSize()-2);
        painter.setFont(tempScaledFont);
        fm = painter.fontMetrics();
        //qDebug() << "TEMP SIZE: " << tempScaledFont.pointSize() << endl;
    }
    //qDebug() << "FM WIDTH NOW: " << fm.width(stick->getName()) << " " << text();

    this->setFont(tempScaledFont);
    QPushButton::paintEvent(event);
    this->setFont(tempWidgetFont);
}
