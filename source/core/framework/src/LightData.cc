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

#include "LightData.hh"
#include "CSException.hh"
#include "VGainFunction.hh"
#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cmath>
#include <numeric>

namespace comptonsoft {
using range_index_t = LightData::range_index_t;
using range_t = LightData::range_t;

LightData::LightData(int n_points, double t_width, double t_start)
{
  setLayout(n_points, t_width, t_start);
}

LightData::~LightData() = default;

void LightData::setLayout(int n_points, double t_width, double t_start)
{
  if (t_width <= 0) {
    BOOST_THROW_EXCEPTION(CSException("t_width must be positive"));
  }
  if (n_points <= 0) {
    BOOST_THROW_EXCEPTION(CSException("n_points must be positive"));
  }
  n_points_ = n_points;
  t_width_ = t_width;
  raw_waveform_.assign(n_points_, 0.0);
  waveform_.assign(n_points_, 0.0);
  resetTimeStart(t_start);
  resetEventData();
}

double LightData::correctGainAtEachBin(double x) const
{
  const auto f = getGainFunction();
  return f ? f->eval(x) : x;
}
void LightData::resetTimeStart(double t_start)
{
  time_.clear();
  t_start_ = t_start;
  for (int i = 0; i < n_points_; ++i) {
    time_.push_back(t_start + t_width_ * i);
  }
}
void LightData::correctGain()
{
  for (int i = 0; i < n_points_; ++i) {
    const double v = raw_waveform_[i];
    waveform_[i] = correctGainAtEachBin(v);
  }
}

double LightData::integral(const range_index_t& range) const
{
  return sum(range) * t_width_;
}

range_index_t LightData::convertRange(const range_t& range) const
{
  const int start = findTimeIndex(range.first);
  const int end = findTimeIndex(range.second);
  return range_index_t{start, end};
}

double LightData::sum(const range_index_t& range) const
{
  int start, end;
  std::tie(start, end) = clampRange(range);
  if (start > end) {
    return 0.0;
  }
  const auto begin = waveform_.begin();
  return std::accumulate(begin + start, begin + end, 0.0);
}

double LightData::integral(const range_t& range) const
{
  return integral(convertRange(range));
}

double LightData::mean(const range_index_t& range) const
{
  int start, end;
  std::tie(start, end) = clampRange(range);
  const int num_points = end - start;
  if (num_points <= 0) {
    return std::nan("");
  }
  return sum(range) / num_points;
}

double LightData::mean(const range_t& range) const
{
  return mean(convertRange(range));
}

int LightData::findTimeIndex(double time) const
{
  return std::clamp(std::ceil((time - t_start_) / t_width_ - 1e-12), -1.0,
                    static_cast<double>(n_points_ + 1)); // if overflow returns n_points + 1, if underflow -1
}

void LightData::subtractPedestal()
{
  for (auto& v : waveform_) {
    v -= pedestal();
  }
}

double LightData::peakPosition(const range_index_t& range) const
{
  int start, end;
  std::tie(start, end) = clampRange(range);
  if (start >= end) {
    return std::nan("");
  }
  const auto begin = waveform_.begin();
  auto iter = std::max_element(begin + start, begin + end);
  const int index = iter - begin;
  return time_[index];
}

double LightData::peak(const range_index_t& range) const
{
  int start, end;
  std::tie(start, end) = clampRange(range);
  if (start >= end) {
    return std::nan("");
  }
  const auto begin = waveform_.begin();
  auto iter = std::max_element(begin + start, begin + end);
  return *iter;
}

double LightData::peak(const range_t& range) const
{
  return peak(convertRange(range));
}

void LightData::resetEventData()
{
  resetRawWaveform();
  resetWaveform();
  pedestal_ = 0.0;
}

range_index_t LightData::clampRange(const range_index_t& range) const
{
  range_index_t ret;
  ret.first = std::clamp(range.first, 0, n_points_);
  ret.second = std::clamp(range.second, 0, n_points_); // if out of range, returns value at the edge.
  return ret;
}

} // namespace comptonsoft