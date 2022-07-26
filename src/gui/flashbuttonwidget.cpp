/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include <QDebug>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QWidget>

FlashButtonWidget::FlashButtonWidget(QWidget *parent)
    : QPushButton(parent)
{
    isflashing = false;
    m_displayNames = false;
    leftAlignText = false;
}

FlashButtonWidget::FlashButtonWidget(bool displayNames, QWidget *parent)
    : QPushButton(parent)
{
    isflashing = false;
    m_displayNames = displayNames;
    leftAlignText = false;
}

void FlashButtonWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);
}

void FlashButtonWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);
}

void FlashButtonWidget::refreshLabel()
{
    QString new_label = generateLabel();
    setText(new_label);

    qDebug() << "label has been set: " << new_label;
}

bool FlashButtonWidget::isButtonFlashing() { return isflashing; }

void FlashButtonWidget::toggleNameDisplay()
{
    m_displayNames = !m_displayNames;
    refreshLabel();
}

void FlashButtonWidget::setDisplayNames(bool display) { m_displayNames = display; }

bool FlashButtonWidget::isDisplayingNames() { return m_displayNames; }

void FlashButtonWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QFont tempScaledFont = painter.font();
    QFont temp;
    tempScaledFont.setPointSize(temp.pointSize());
    QFontMetrics fm(tempScaledFont);

    bool reduce = false;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    while ((this->width() < fm.horizontalAdvance(text())) && (tempScaledFont.pointSize() >= 7))
#else
    while ((this->width() < fm.width(text())) && (tempScaledFont.pointSize() >= 7))
#endif
    {
        tempScaledFont.setPointSize(tempScaledFont.pointSize() - 1);
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
        } else if (!reduce && leftAlignText)
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

void FlashButtonWidget::retranslateUi() { refreshLabel(); }

bool FlashButtonWidget::ifDisplayNames() { return m_displayNames; }
