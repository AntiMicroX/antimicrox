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

#include <QListWidgetItem>

#include "slotitemlistwidget.h"
#include "simplekeygrabberbutton.h"

SlotItemListWidget::SlotItemListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

void SlotItemListWidget::keyPressEvent(QKeyEvent *event)
{
    bool propogate = true;

    QListWidgetItem *currentItem = this->item(this->currentRow());
    SimpleKeyGrabberButton *tempbutton = 0;
    if (currentItem)
    {
        tempbutton = currentItem->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();
    }

    if (tempbutton && tempbutton->isGrabbing())
    {
        switch (event->key())
        {
            case Qt::Key_Home:
            case Qt::Key_End:
            {
                propogate = false;
                break;
            }
        }
    }

    if (propogate)
    {
        QListWidget::keyPressEvent(event);
    }
}
