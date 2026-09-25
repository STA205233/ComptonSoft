/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#include "NanoGRAMSTemperatureCorrection.hh"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <boost/throw_exception.hpp>
#include "CSException.hh"

namespace comptonsoft {

NanoGRAMSTemperatureCorrection::NanoGRAMSTemperatureCorrection(int numFECs)
  : referenceADC_(numFECs, std::numeric_limits<double>::quiet_NaN()),
    fixedTestPulseADC_(numFECs, std::numeric_limits<double>::quiet_NaN()),
    testPulseTimes_(numFECs),
    testPulseADCs_(numFECs)
{
}

NanoGRAMSTemperatureCorrection::~NanoGRAMSTemperatureCorrection() = default;

void NanoGRAMSTemperatureCorrection::setFixedTestPulseADC(int fec, double v)
{
  if (!(v > 0.0)) {
    BOOST_THROW_EXCEPTION(CSException("Test-pulse ADC value must be positive."));
  }
  fixedTestPulseADC_.at(fec) = v;
}

void NanoGRAMSTemperatureCorrection::addTestPulseADC(int fec, double unixTime, double v)
{
  if (!(v > 0.0)) {
    BOOST_THROW_EXCEPTION(CSException("Test-pulse ADC value must be positive."));
  }
  std::vector<double>& times = testPulseTimes_.at(fec);
  std::vector<double>& values = testPulseADCs_.at(fec);
  const auto position = std::upper_bound(times.begin(), times.end(), unixTime);
  const auto index = std::distance(times.begin(), position);
  times.insert(position, unixTime);
  values.insert(values.begin() + index, v);
}

double NanoGRAMSTemperatureCorrection::measuredTestPulseADC(int fec, double unixTime) const
{
  const std::vector<double>& times = testPulseTimes_.at(fec);
  const std::vector<double>& values = testPulseADCs_.at(fec);
  if (times.empty()) {
    return fixedTestPulseADC_.at(fec);
  }

  // linear interpolation; out of the range of the time series -> NaN
  if (unixTime < times.front() || unixTime > times.back()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const auto after = std::lower_bound(times.begin(), times.end(), unixTime);
  const std::size_t i1 = std::distance(times.begin(), after);
  if (times[i1] == unixTime || i1 == 0) {
    return values[i1];
  }
  const std::size_t i0 = i1 - 1;
  const double weight = (unixTime - times[i0]) / (times[i1] - times[i0]);
  return (1.0 - weight) * values[i0] + weight * values[i1];
}

double NanoGRAMSTemperatureCorrection::factor(int fec, double unixTime) const
{
  const bool measured = !testPulseTimes_.at(fec).empty() || std::isfinite(fixedTestPulseADC_.at(fec));
  if (!measured) {
    return 1.0;
  }
  return referenceADC_.at(fec) / measuredTestPulseADC(fec, unixTime);
}

} // namespace comptonsoft
