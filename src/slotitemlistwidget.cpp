/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "slotitemlistwidget.h"

#include "messagehandler.h"
#include "simplekeygrabberbutton.h"

#include <QListWidgetItem>
#include <QKeyEvent>
#include <QWidget>
#include <QDebug>



SlotItemListWidget::SlotItemListWidget(QWidget *parent) :
    QListWidget(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

}

void SlotItemListWidget::insertItems(int row, QList<QListWidgetItem *> items)
{
    for(auto el : items)
    {
        insertItem(row, el);
        row++;
    }
}

void SlotItemListWidget::keyPressEvent(QKeyEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool propogate = true;

    QListWidgetItem *currentItem = this->item(this->currentRow());
    SimpleKeyGrabberButton *tempbutton = nullptr;

    if (currentItem != nullptr)
        tempbutton = currentItem->data(Qt::UserRole).value<SimpleKeyGrabberButton*>();

    if (tempbutton != nullptr && tempbutton->isGrabbing())
    {
        switch (event->key())
        {
            case Qt::Key_Home:
            case Qt::Key_End:
            {
                propogate = false;
                break;
            }

            default:
            break;
        }
    }

    if (propogate) QListWidget::keyPressEvent(event);
}
