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

    setText(generateLabel());

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(axis, SIGNAL(throttleChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
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
    disconnect(axis, SIGNAL(active(int)), 0, 0);
    disconnect(axis, SIGNAL(released(int)), 0, 0);
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
    temp = axis->getName().replace("&", "&&");
    return temp;
}

void JoyAxisWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QFontMetrics fm = this->fontMetrics();
    QFont tempWidgetFont = this->font();
    QFont tempScaledFont = painter.font();

    while ((this->width() < fm.width(text())) && tempScaledFont.pointSize() >= 6)
    {
        tempScaledFont.setPointSize(painter.font().pointSize()-2);
        painter.setFont(tempScaledFont);
        fm = painter.fontMetrics();
    }

    this->setFont(tempScaledFont);
    QPushButton::paintEvent(event);
    this->setFont(tempWidgetFont);
}
