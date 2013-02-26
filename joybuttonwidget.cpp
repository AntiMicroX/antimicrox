#include <QDebug>
#include <QStyle>

#include "joybuttonwidget.h"
#include "event.h"

JoyButtonWidget::JoyButtonWidget(QWidget *parent) :
    QPushButton(parent)
{

    normal = this->palette();
    flashing = this->palette();

    QColor highlightColor = QColor(0, 0, 255);
    flashing.setCurrentColorGroup(QPalette::Inactive);
    flashing.setColor(QPalette::Button, highlightColor);
    flashing.setColor(QPalette::Light, highlightColor.light(150));
    flashing.setColor(QPalette::Midlight, highlightColor.light(125));
    flashing.setColor(QPalette::Dark, highlightColor.dark(200));
    flashing.setColor(QPalette::Mid, highlightColor.dark(150));

    setPalette(flashing);
    isflashing = false;
}

JoyButtonWidget::JoyButtonWidget(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{

    this->button = button;

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

    setText(button->getName());

    connect (button, SIGNAL(keyChanged(int)), this, SLOT(changeKeyLabel()));
    connect (button, SIGNAL(mouseChanged(int)), this, SLOT(changeMouseLabel()));
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
    //setPalette(flashing);
    //update();
    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::unflash()
{
    isflashing = false;
    //setPalette(normal);
    //update();

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::changeKeyLabel()
{
    setText(button->getName());
}

void JoyButtonWidget::changeMouseLabel()
{
    setText(button->getName());
}

void JoyButtonWidget::refreshLabel()
{
    setText(button->getName());
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
