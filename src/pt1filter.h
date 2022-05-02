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

/**
 * @brief Implementation of a first order lag or PT1 filter.
 *  Can be used for example to smooth noisy values a bit or detect envelopes.
 * @see https://en.wikipedia.org/wiki/Low-pass_filter
 */
class PT1Filter
{
  public:
    PT1Filter(double tau = 1, double rate = 1);

    double process(double value);
    /**
     * @brief Get the current filter output value.
     * @returns Current filter output value.
     */
    inline double getValue() const { return m_value; }
    void reset();

    static const double FALLBACK_RATE;

  private:
    double m_dt_tau;
    double m_value;
};
