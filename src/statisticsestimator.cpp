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

#include "statisticsestimator.h"

StatisticsEstimator::StatisticsEstimator() { reset(); }

/**
 * @brief Resets the StatisticsEstimator so that it is ready to process a new data stream.
 */
void StatisticsEstimator::reset()
{
    m_mean = 0;
    m_var = 0;
    m_count = 0;
}

/**
 * @brief Processes a new sample of the current data stream and updates internal
 *  intermediate values.
 */
void StatisticsEstimator::process(double x)
{
    ++m_count;
    double dx = x - m_mean;
    m_mean += dx / m_count;
    double dx2 = x - m_mean;
    m_var += dx * dx2;
}

/**
 * @brief Calculates the sample variance of the processed data stream from
 *  internal intermediate values.
 * @returns Sample variance
 */
double StatisticsEstimator::calculateVariance() const { return m_var / (m_count - 1); }

/**
 * @brief Calculates the squared relative three sigma error range, i.e. the squared
 *  estimation accuracy in percent,  of the processed data stream from internal
 *  intermediate values. Use the squared value to avoid expensive root calculation.
 * @returns Squared relative error range.
 */
double StatisticsEstimator::calculateRelativeErrorSq() const
{
    return 9 * calculateVariance() / (m_count * m_mean * m_mean);
}
