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

#include "pt1filter.h"
#include <Qt>

const double PT1Filter::FALLBACK_RATE = 200;

PT1Filter::PT1Filter(double tau, double rate)
{
    double period = qFuzzyIsNull(rate) ? 1 / FALLBACK_RATE : 1 / rate;
    // Since it is a fixed rate filter, precalculte delta_t/tau
    // to save one division during sample processing.
    m_dt_tau = period / tau;
    reset();
}

/**
 * @brief Processes a new sample.
 * @returns New filter output value.
 */
double PT1Filter::process(double value)
{
    m_value = m_value + m_dt_tau * (value - m_value);
    return m_value;
};

/**
 * @brief Resets the filter state to default.
 */
void PT1Filter::reset() { m_value = 0; }
