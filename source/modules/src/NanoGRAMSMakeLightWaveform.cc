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

#include "NanoGRAMSMakeLightWaveform.hh"

#include "AstroUnits.hh"
#include "NanoGRAMSHitExtraction.hh"
#include "NanoGRAMSLightWaveformStore.hh"
#include "NanoGRAMSTPCTreeIO.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

using namespace anlnext;

namespace comptonsoft
{
using namespace grams;

NanoGRAMSMakeLightWaveform::NanoGRAMSMakeLightWaveform() = default;

NanoGRAMSMakeLightWaveform::~NanoGRAMSMakeLightWaveform() = default;

ANLStatus NanoGRAMSMakeLightWaveform::mod_define()
{
  define_parameter("hit_extraction_module_name",       &mod_class::hitExtractionModuleName_);
  define_parameter("light_waveform_store_module_name", &mod_class::lightWaveformStoreModuleName_);
  define_parameter("range_min_us",                     &mod_class::rangeMinUs_);
  define_parameter("range_max_us",                     &mod_class::rangeMaxUs_);
  return AS_OK;
}

ANLStatus NanoGRAMSMakeLightWaveform::mod_initialize()
{
  const ANLStatus status = BasicModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (exist_module(hitExtractionModuleName_)) {
    get_module_NC(hitExtractionModuleName_, &hitExtraction_);
  }
  else {
    std::cerr << "NanoGRAMSMakeLightWaveform::mod_initialize: "
              << hitExtractionModuleName_ << " module not found" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  if (exist_module(lightWaveformStoreModuleName_)) {
    get_module_NC(lightWaveformStoreModuleName_, &lightWaveformStore_);
  }
  else {
    std::cerr << "NanoGRAMSMakeLightWaveform::mod_initialize: "
              << lightWaveformStoreModuleName_ << " module not found" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  return AS_OK;
}

ANLStatus NanoGRAMSMakeLightWaveform::mod_analyze()
{
  namespace unit = anlgeant4::unit;

  const TPCTreeBuffer& tpc_tree_buffer = hitExtraction_->currentBuffer();
  const Config& cfg = hitExtraction_->config();
  const LightStatus& light_status = hitExtraction_->currentLightStatus();
  const TPCTreeLayout& layout = tpc_tree_buffer.layout();

  struct ChannelWindow
  {
    int light_ch = 0;
    const std::vector<double>* waveform = nullptr;
    int waveform_len = 0;
    double bin_width_us = 0.0;  // one sample, in us
    double xmin_full_us = 0.0;  // time of sample 0, in us
    int valid_stop_idx = 0;     // wave_num: samples [0, valid_stop_idx) are real data
  };
  std::vector<ChannelWindow> channels;

  double trim_min = std::numeric_limits<double>::infinity();
  double trim_max = -std::numeric_limits<double>::infinity();

  for (int light_ch = 0; light_ch < layout.num_dpp_registered_slots; ++light_ch) {
    if (!tpc_tree_buffer.registered_channels[light_ch]) {
      continue;
    }
    if (!light_status.corrected_waveform_valid[light_ch]) {
      continue;
    }
    const std::vector<double>& waveform = light_status.corrected_waveform[light_ch];
    if (waveform.empty()) {
      continue;
    }
    const int waveform_len = static_cast<int>(waveform.size());

    const double dt = static_cast<double>(tpc_tree_buffer.wave_compress[light_ch]) * unit::ns;
    const double trigger_delay =
        static_cast<double>(cfg.light_delay_counts[light_ch]) * 8.0 * dt;
    const double bin_width_us = dt / unit::us;
    const double xmin_full_us = -trigger_delay / unit::us;

    const int valid_stop_idx =
        std::clamp(static_cast<int>(tpc_tree_buffer.wave_num[light_ch]), 0, waveform_len);

    channels.push_back(
        {light_ch, &waveform, waveform_len, bin_width_us, xmin_full_us, valid_stop_idx});

    if (!hasFixedRange() && valid_stop_idx > 0) {
      trim_min = std::min(trim_min, xmin_full_us);
      trim_max = std::max(trim_max, xmin_full_us + valid_stop_idx * bin_width_us);
    }
  }

  if (channels.empty()) {
    return AS_OK;
  }

  const bool has_trim_range = trim_min < trim_max;
  double crop_min = -std::numeric_limits<double>::infinity();
  double crop_max = std::numeric_limits<double>::infinity();
  if (hasFixedRange()) {
    crop_min = rangeMinUs_;
    crop_max = rangeMaxUs_;
  } else if (has_trim_range) {
    crop_min = trim_min;
    crop_max = trim_max;
  }

  const int64_t raw_event_id = hitExtraction_->currentRawEventId();

  for (const ChannelWindow& channel : channels) {
    const int start_idx = std::clamp(
        static_cast<int>(std::floor((crop_min - channel.xmin_full_us) / channel.bin_width_us)),
        0, channel.waveform_len);
    const int stop_idx = std::clamp(
        static_cast<int>(std::ceil((crop_max - channel.xmin_full_us) / channel.bin_width_us)),
        start_idx, std::min(channel.waveform_len, channel.valid_stop_idx));
    const int nbins = stop_idx - start_idx;
    if (nbins <= 0) {
      continue;
    }

    LightWaveform stored;
    stored.xlow_us = channel.xmin_full_us + start_idx * channel.bin_width_us;
    stored.xhigh_us = channel.xmin_full_us + stop_idx * channel.bin_width_us;
    stored.values.assign(channel.waveform->begin() + start_idx,
                         channel.waveform->begin() + stop_idx);

    lightWaveformStore_->push(raw_event_id, channel.light_ch, stored);
  }

  return AS_OK;
}

} /* namespace comptonsoft */
