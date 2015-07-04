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

#include <QLabel>
#include <QVBoxLayout>

#include "firstrunwelcomepage.h"

FirstRunWelcomePage::FirstRunWelcomePage(AntiMicroSettings *settings, QWidget *parent) :
    QWizardPage(parent)
{
    this->settings = settings;

    setTitle(tr("Welcome"));
    setLayout(new QVBoxLayout);

    QLabel *tempLabel = new QLabel(
                tr("Thank you for checking out antimicro. This "
                   "wizard can be used to customize some of the "
                   "program's behavior. More settings can be found "
                   "from the main interface under "
                   "Options > Settings."));
    tempLabel->setWordWrap(true);
    layout()->addWidget(tempLabel);
}
