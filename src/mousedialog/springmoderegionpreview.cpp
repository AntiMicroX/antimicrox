/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <QDebug>
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
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    setAttribute(Qt::WA_PaintOnScreen);
#endif

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
