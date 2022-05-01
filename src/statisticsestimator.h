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

#include <cstddef>

/**
 * @brief Estimates mean of a data stream using Welford's algorithm and
 *  calculates statistic properties.
 */
class StatisticsEstimator
{
  public:
    StatisticsEstimator();

    void reset();
    void process(double x);

    /**
     * @brief Gets the amount of processed samples.
     * @returns Processed sample count.
     */
    inline size_t getCount() const { return m_count; }
    /**
     * @brief Gets the mean of processed samples.
     * @returns Mean of processed samples.
     */
    inline double getMean() const { return m_mean; }
    double calculateVariance() const;
    double calculateRelativeErrorSq() const;

  private:
    double m_mean;
    double m_var;
    size_t m_count;
};
