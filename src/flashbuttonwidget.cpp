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

#include "flashbuttonwidget.h"

#include "messagehandler.h"

#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QWidget>


FlashButtonWidget::FlashButtonWidget(QWidget *parent) :
    QPushButton(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    isflashing = false;
    displayNames = false;
    leftAlignText = false;
}

FlashButtonWidget::FlashButtonWidget(bool displayNames, QWidget *parent) :
    QPushButton(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    isflashing = false;
    this->displayNames = displayNames;
    leftAlignText = false;
}

void FlashButtonWidget::flash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void FlashButtonWidget::unflash()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void FlashButtonWidget::refreshLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setText(generateLabel());

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "label has been set: " << generateLabel();
    #endif
}

bool FlashButtonWidget::isButtonFlashing()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return isflashing;
}

void FlashButtonWidget::toggleNameDisplay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    displayNames = !displayNames;
    refreshLabel();
}

void FlashButtonWidget::setDisplayNames(bool display)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    displayNames = display;
}

bool FlashButtonWidget::isDisplayingNames()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return displayNames;
}

void FlashButtonWidget::paintEvent(QPaintEvent *event)
{
   // qInstallMessageHandler(MessageHandler::myMessageOutput);

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

void FlashButtonWidget::retranslateUi()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    refreshLabel();
}

bool FlashButtonWidget::ifDisplayNames() {

    return displayNames;
}
