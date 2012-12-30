#include <QDebug>

#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(QWidget *parent) :
    QPushButton(parent)
{
    axis = 0;

    normal = this->palette();
    flashing = this->palette();
    QColor highlightColor = QColor(0, 0, 255);

    flashing.setCurrentColorGroup(QPalette::Inactive);
    flashing.setColor(QPalette::Button, highlightColor);
    flashing.setColor(QPalette::Light, highlightColor.light(150));
    flashing.setColor(QPalette::Midlight, highlightColor.light(125));
    flashing.setColor(QPalette::Dark, highlightColor.dark(200));
    flashing.setColor(QPalette::Mid, highlightColor.dark(150));

    setPalette(normal);
    isflashing = false;
}

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, QWidget *parent) :
    QPushButton(parent)
{
    this->axis = axis;

    normal = this->palette();
    flashing = this->palette();
    QColor highlightColor = QColor(0, 0, 255);

    flashing.setCurrentColorGroup(QPalette::Inactive);
    flashing.setColor(QPalette::Button, highlightColor);
    flashing.setColor(QPalette::Light, highlightColor.light(150));
    flashing.setColor(QPalette::Midlight, highlightColor.light(125));
    flashing.setColor(QPalette::Dark, highlightColor.dark(200));
    flashing.setColor(QPalette::Mid, highlightColor.dark(150));

    setPalette(normal);
    isflashing = false;

    setText(axis->getName());

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::flash()
{
    isflashing = true;
    setPalette(flashing);
    emit flashed(isflashing);
}

void JoyAxisWidget::unflash()
{
    isflashing = false;
    setPalette(normal);
    emit flashed(isflashing);
}

void JoyAxisWidget::refreshLabel()
{
    setText(axis->getName());
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
