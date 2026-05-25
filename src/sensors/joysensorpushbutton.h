/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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
#pragma once

#include "flashbuttonwidget.h"

class JoySensor;
class QWidget;

/**
 * @brief The central button in a SensorPushButtonGroup
 */
class JoySensorPushButton : public FlashButtonWidget
{
    Q_OBJECT

  public:
    explicit JoySensorPushButton(JoySensor *sensor, bool displayNames, QWidget *parent = nullptr);

    JoySensor *getSensor() const;
    void tryFlash();

  protected:
    virtual QString generateLabel() override;

  public slots:
    void disableFlashes() override;
    void enableFlashes() override;

  private slots:
    void showContextMenu(const QPoint &point);

  private:
    JoySensor *m_sensor;
};
