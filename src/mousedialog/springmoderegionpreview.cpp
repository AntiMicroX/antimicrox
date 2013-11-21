#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>

#include "springmoderegionpreview.h"

SpringModeRegionPreview::SpringModeRegionPreview(int width, int height, QWidget *parent) :
    #if defined(Q_OS_WIN)
        QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    #else
        QWidget(parent, Qt::FramelessWindowHint)
    #endif
{
    int tempwidth = adjustSpringSizeWidth(width);
    int tempheight = adjustSpringSizeHeight(height);

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setWindowTitle(tr("Spring Mode Preview"));

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

    p.drawRect(1, 1, width()-3, height()-3);
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

#ifdef Q_OS_UNIX
    hide();
#endif

    if (tempwidth >= 2 && height >= 2)
    {
        int cw = (qApp->desktop()->width() / 2) - (tempwidth / 2);
        int ch = (qApp->desktop()->height() / 2) - (height / 2);

        setGeometry(cw, ch, tempwidth, height);
        if (!isVisible())
        {
            show();
        }
    }
    else
    {
#ifndef Q_OS_UNIX
        hide();
#endif
        resize(tempwidth, height);
        move(0, 0);
    }
}

void SpringModeRegionPreview::setSpringHeight(int height)
{
    int tempheight = adjustSpringSizeHeight(height);

    int width = size().width();

#ifdef Q_OS_UNIX
    hide();
#endif

    if (width >= 2 && tempheight >= 2)
    {
        int cw = (qApp->desktop()->width() / 2) - (width / 2);
        int ch = (qApp->desktop()->height() / 2) - (tempheight / 2);

        setGeometry(cw, ch, width, tempheight);
        if (!isVisible())
        {
            show();
        }
    }
    else
    {
#ifndef Q_OS_UNIX
        hide();
#endif
        resize(width, tempheight);
        move(0, 0);
    }
}

void SpringModeRegionPreview::setSpringSize(int width, int height)
{
    int tempwidth = adjustSpringSizeWidth(width);
    int tempheight = adjustSpringSizeHeight(height);

    int cw = (qApp->desktop()->width() / 2) - (tempwidth / 2);
    int ch = (qApp->desktop()->height() / 2) - (height / 2);

    resize(tempwidth, tempheight);
    move(cw, ch);
}
