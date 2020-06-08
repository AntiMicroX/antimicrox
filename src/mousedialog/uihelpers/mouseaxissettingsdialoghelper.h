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


#ifndef MOUSEAXISSETTINGSDIALOGHELPER_H
#define MOUSEAXISSETTINGSDIALOGHELPER_H

#include <QObject>

class JoyAxis;

class MouseAxisSettingsDialogHelper : public QObject
{
    Q_OBJECT

public:
    explicit MouseAxisSettingsDialogHelper(JoyAxis *axis, QObject *parent = nullptr);

    JoyAxis* getAxis() const;

public slots:
    void updateExtraAccelerationStatus(bool checked);
    void updateExtraAccelerationMultiplier(double value);

    void updateStartMultiPercentage(double value);
    void updateMinAccelThreshold(double value);
    void updateMaxAccelThreshold(double value);
    void updateAccelExtraDuration(double value);
    void updateReleaseSpringRadius(int value);

private:
    JoyAxis *axis;

};

#endif // MOUSEAXISSETTINGSDIALOGHELPER_H
