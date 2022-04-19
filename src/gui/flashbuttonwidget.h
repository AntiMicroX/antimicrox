/* antimicrox Gamepad to KB+M event mapper
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

#ifndef FLASHBUTTONWIDGET_H
#define FLASHBUTTONWIDGET_H

#include <QPushButton>

class QWidget;
class QPaintEvent;

class FlashButtonWidget : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool isflashing READ isButtonFlashing)

  public:
    explicit FlashButtonWidget(QWidget *parent = nullptr);
    explicit FlashButtonWidget(bool displayNames, QWidget *parent = nullptr);

    bool isButtonFlashing();
    void setDisplayNames(bool display);
    bool isDisplayingNames();

  protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual QString generateLabel() = 0;
    virtual void retranslateUi();
    bool ifDisplayNames();

  public slots:
    void refreshLabel();
    void toggleNameDisplay();
    virtual void disableFlashes() = 0;
    virtual void enableFlashes() = 0;

  protected slots:
    void flash();
    void unflash();

  private:
    bool isflashing;
    bool m_displayNames;
    bool leftAlignText;
};

#endif // FLASHBUTTONWIDGET_H
