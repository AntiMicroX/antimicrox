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

#ifndef FLASHBUTTONWIDGET_H
#define FLASHBUTTONWIDGET_H

#include <QPushButton>
#include <QPaintEvent>

class FlashButtonWidget : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

public:
    explicit FlashButtonWidget(QWidget *parent = 0);
    explicit FlashButtonWidget(bool displayNames, QWidget *parent = 0);

    bool isButtonFlashing();
    void setDisplayNames(bool display);
    bool isDisplayingNames();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual QString generateLabel() = 0;
    virtual void retranslateUi();

    bool isflashing;
    bool displayNames;
    bool leftAlignText;

signals:
    void flashed(bool flashing);

public slots:
    void refreshLabel();
    void toggleNameDisplay();
    virtual void disableFlashes() = 0;
    virtual void enableFlashes() = 0;

protected slots:
    void flash();
    void unflash();
};

#endif // FLASHBUTTONWIDGET_H
