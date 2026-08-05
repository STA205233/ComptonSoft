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

#include "NanoGRAMSGetPeak.hh"

#include "AstroUnits.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "NanoGRAMSLightWaveformStore.hh"

#include <algorithm>
#include <cstddef>
#include <limits>

using namespace anlnext;
using namespace anlgeant4::unit;

namespace comptonsoft {
NanoGRAMSGetPeak::NanoGRAMSGetPeak() = default;
NanoGRAMSGetPeak::~NanoGRAMSGetPeak() = default;

ANLStatus NanoGRAMSGetPeak::mod_define()
{
  VNanoGRAMSLightWaveformQuery::mod_define();
  define_parameter("x_min", &mod_class::x_min_, CLHEP::us, "us");
  define_parameter("x_max", &mod_class::x_max_, CLHEP::us, "us");
  define_parameter("set_to_hit", &mod_class::set_to_hit_);
  return AS_OK;
}

ANLStatus NanoGRAMSGetPeak::mod_initialize()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (channels_.empty()) {
    std::cerr << module_id() << ": no light analysis channels configured" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }
  setHistValueName("Peak Value [mV]");

  channel_list_ = channels_;
  peaks_.assign(channel_list_.size(), std::numeric_limits<double>::quiet_NaN());
  peak_pos_.assign(channel_list_.size(), std::numeric_limits<double>::quiet_NaN());

  general_channel_.assign(channel_list_.size(), false);
  for (std::size_t index = 0; index < channel_list_.size(); index++) {
    const std::vector<int>& general = cfg_.general_analysis_channels;
    general_channel_[index] = std::find(general.begin(), general.end(), channel_list_[index]) != general.end();
  }

  if (set_to_hit_) {
    if (!exist_module("CSHitCollection")) {
      std::cerr << module_id() << ": set_to_hit requires CSHitCollection "
                << "in the same ANL chain." << std::endl;
      return AS_QUIT_ALL_ERROR;
    }
    get_module_NC("CSHitCollection", &hit_collection_);
  }

  return AS_OK;
}

ANLStatus NanoGRAMSGetPeak::mod_analyze()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_analyze();
  resetHistValue();
  std::fill(peaks_.begin(), peaks_.end(), std::numeric_limits<double>::quiet_NaN());
  std::fill(peak_pos_.begin(), peak_pos_.end(), std::numeric_limits<double>::quiet_NaN());
  if (status != AS_OK) {
    return status;
  }
  if (isSkipLoop()) {
    return AS_OK;
  }

  find_peak();

  if (set_to_hit_ && Valid()) {
    apply_to_hits();
  }

  return AS_OK;
}

void NanoGRAMSGetPeak::apply_to_hits()
{
  const double value = HistValue();
  const int num_time_groups = hit_collection_->NumberOfTimeGroups();
  for (int g = 0; g < num_time_groups; ++g) {
    for (auto& hit : hit_collection_->getHits(g)) {
      hit->setEPI(value * keV); // EPI uses keV in default
    }
  }
}

void NanoGRAMSGetPeak::find_peak()
{
  namespace unit = anlgeant4::unit;
  constexpr double millivolt = unit::volt / 1000.0;
  const double x_min_us = x_min_ / unit::us;
  const double x_max_us = x_max_ / unit::us;
  bool has_valid_channel = false;
  double total_peak = 0;

  for (std::size_t index = 0; index < channel_list_.size(); index++) {
    const int channel = channel_list_[index];
    if (!lightWaveformStore_->isValid(channel)) {
      continue;
    }

    const grams::LightWaveform& waveform = lightWaveformStore_->waveform(channel);
    const std::size_t nbins = waveform.values.size();
    if (nbins == 0) {
      continue;
    }

    const double bin_width_us = (waveform.xhigh_us - waveform.xlow_us) / static_cast<double>(nbins);
    if (bin_width_us <= 0.0) {
      continue;
    }

    double peak = std::numeric_limits<double>::min();
    double peak_pos = 0;
    for (std::size_t i = 0; i < nbins; ++i) {
      const double center_us = waveform.xlow_us + (static_cast<double>(i) + 0.5) * bin_width_us;
      if (center_us < x_min_us) {
        continue;
      }
      else if (center_us >= x_max_us) {
        break;
      }
      const double value_mV = waveform.values[i] / millivolt;
      if (peak < value_mV) {
        peak = value_mV;
        peak_pos = center_us;
      }
    }

    if (general_channel_[index]) {
      has_valid_channel = true;
      total_peak += peak;
      peaks_[index] = peak;
      peak_pos_[index] = peak_pos;
    }
  }

  if (!has_valid_channel) {
    return;
  }

  setHistValue(total_peak);
}

} // namespace comptonsoft
