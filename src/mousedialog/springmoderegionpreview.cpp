#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>

#include "springmoderegionpreview.h"

SpringModeRegionPreview::SpringModeRegionPreview(int width, int height, QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint | Qt::SplashScreen)
{
    int tempwidth = adjustSpringSizeWidth(width);
    int tempheight = adjustSpringSizeHeight(height);

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_PaintOnScreen);

    if (tempwidth >= 2 && tempheight >= 2)
    {
        int cw = (qApp->desktop()->width() / 2) - (tempwidth / 2);
        int ch = (qApp->desktop()->height() / 2) - (tempheight / 2);

        setGeometry(cw, ch, tempwidth, tempheight);
        show();
    }
    else
    {
        resize(0, 0);
        move(0, 0);
    }
}

void SpringModeRegionPreview::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);

    QPainter p(this);

    QPen border;
    border.setWidth(3);
    border.setColor(Qt::black);
    p.setPen(border);

    p.drawRect(3, 3, width()-6, height()-6);

    QWidget::paintEvent(event);
}

int SpringModeRegionPreview::adjustSpringSizeWidth(int width)
{
    int tempwidth = size().width();
    if (width >= 2)
    {
        tempwidth = width;
    }
    else
    {
        tempwidth = 0;
    }

    return tempwidth;
}

int SpringModeRegionPreview::adjustSpringSizeHeight(int height)
{
    int tempheight = size().height();

    if (height >= 2)
    {
        tempheight = height;
    }
    else
    {
        tempheight = 0;
    }

    return tempheight;
}

void SpringModeRegionPreview::setSpringWidth(int width)
{
    int tempwidth = adjustSpringSizeWidth(width);

    int height = size().height();

    hide();
    if (tempwidth >= 2 && height >= 2)
    {
        int cw = (qApp->desktop()->width() / 2) - (tempwidth / 2);
        int ch = (qApp->desktop()->height() / 2) - (height / 2);

        setGeometry(cw, ch, tempwidth, height);
        show();
    }
    else if (tempwidth >= 2)
    {
        resize(tempwidth, height);
        move(0, 0);
    }
}

void SpringModeRegionPreview::setSpringHeight(int height)
{
    int tempheight = adjustSpringSizeHeight(height);

    int width = size().width();

    hide();
    if (width >= 2 && tempheight >= 2)
    {
        int cw = (qApp->desktop()->width() / 2) - (width / 2);
        int ch = (qApp->desktop()->height() / 2) - (tempheight / 2);

        setGeometry(cw, ch, width, tempheight);
        show();

    }
    else if (tempheight >= 2)
    {
        resize(width, tempheight);
        move(0, 0);
    }
}
