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

#include "NanoGRAMSReadDppList.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "TChain.h"
#include "TFile.h"

#include "AstroUnits.hh"
#include "NanoGRAMSDppListDataProcessor.hh"
#include "NanoGRAMSLightAnalysis.hh"
#include "NanoGRAMSLightWaveform.hh"
#include "NanoGRAMSLightWaveformStore.hh"

using namespace anlnext;

namespace comptonsoft {
using namespace grams;

NanoGRAMSReadDppList::NanoGRAMSReadDppList()
  : chain_(nullptr),
    lightWaveformStoreModuleName_("NanoGRAMSLightWaveformStore")
{
}

NanoGRAMSReadDppList::~NanoGRAMSReadDppList() = default;

ANLStatus NanoGRAMSReadDppList::mod_define()
{
  define_parameter("dpplist_file", &mod_class::dppListFile_);
  define_parameter("config_file", &mod_class::configFile_);
  define_parameter("light_waveform_store_module_name", &mod_class::lightWaveformStoreModuleName_);
  define_parameter("version", &mod_class::version_);
  define_parameter("verbose", &mod_class::verbose_);
  define_parameter("range_min_us", &mod_class::rangeMinUs_);
  define_parameter("range_max_us", &mod_class::rangeMaxUs_);
  return AS_OK;
}

ANLStatus NanoGRAMSReadDppList::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (dppListFile_.empty()) {
    std::cerr << module_id() << ": dpplist_file is not given." << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  if (exist_module(lightWaveformStoreModuleName_)) {
    get_module_NC(lightWaveformStoreModuleName_, &lightWaveformStore_);
  }
  else {
    std::cerr << module_id() << ": " << lightWaveformStoreModuleName_
              << " module not found" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  grams::readConfig(cfg_, configFile_);

  grams::readDPPConfig(cfg_, dppListFile_[0]);

  chain_ = std::make_unique<TChain>("dpplist");
  for (const auto& filename : dppListFile_) {
    chain_->Add(filename.c_str());
  }

  reader_ = std::make_unique<DppListTreeReader>(chain_.get(), version_, verbose_);
  std::cout << module_id() << ": " << reader_->nEntries()
            << " dpplist entries" << std::endl;

  return AS_OK;
}

ANLStatus NanoGRAMSReadDppList::mod_analyze()
{
  namespace unit = anlgeant4::unit;

  if (!reader_->processNext()) {
    std::cout << "[" << module_id() << "] AS_QUIT after processing "
              << processedEvents_ << " events." << std::endl;
    return AS_QUIT;
  }
  ++processedEvents_;

  struct ChannelWindow
  {
    int light_ch = 0;
    std::vector<double> waveform;
    double bin_width_us = 0.0; // one sample, in us
    double xmin_full_us = 0.0; // time of sample 0, in us
  };
  std::vector<ChannelWindow> channels;

  double trim_min = std::numeric_limits<double>::infinity();
  double trim_max = -std::numeric_limits<double>::infinity();

  for (int light_ch = 0; light_ch < NUM_CH_DPP_MAX; ++light_ch) {
    if (!reader_->isRegistered(light_ch)) {
      continue;
    }

    const LightWaveformView view = viewFromDppListHit(reader_->hit(light_ch));
    std::vector<double> waveform = lightVoltageWaveformFromView(cfg_, view);
    if (waveform.empty()) {
      continue;
    }
    const int waveform_len = static_cast<int>(waveform.size());

    const double dt = static_cast<double>(view.wave_compress) * unit::ns;
    const double trigger_delay =
        static_cast<double>(cfg_.light_delay_counts[light_ch]) * 8.0 * dt;
    const double bin_width_us = dt / unit::us;
    const double xmin_full_us = -trigger_delay / unit::us;

    channels.push_back({light_ch, std::move(waveform), bin_width_us, xmin_full_us});

    if (!hasFixedRange()) {
      trim_min = std::min(trim_min, xmin_full_us);
      trim_max = std::max(trim_max, xmin_full_us + waveform_len * bin_width_us);
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
  }
  else if (has_trim_range) {
    crop_min = trim_min;
    crop_max = trim_max;
  }

  const int64_t raw_event_id = static_cast<int64_t>(reader_->currentTriggerId());

  for (const ChannelWindow& channel : channels) {
    const int waveform_len = static_cast<int>(channel.waveform.size());
    const int start_idx = std::clamp(
        static_cast<int>(std::floor((crop_min - channel.xmin_full_us) / channel.bin_width_us)),
        0, waveform_len);
    const int stop_idx = std::clamp(
        static_cast<int>(std::ceil((crop_max - channel.xmin_full_us) / channel.bin_width_us)),
        start_idx, waveform_len);
    const int nbins = stop_idx - start_idx;
    if (nbins <= 0) {
      continue;
    }

    LightWaveform stored;
    stored.xlow_us = channel.xmin_full_us + start_idx * channel.bin_width_us;
    stored.xhigh_us = channel.xmin_full_us + stop_idx * channel.bin_width_us;
    stored.values.assign(channel.waveform.begin() + start_idx,
                         channel.waveform.begin() + stop_idx);

    lightWaveformStore_->push(raw_event_id, channel.light_ch, stored);
  }

  return AS_OK;
}

ANLStatus NanoGRAMSReadDppList::mod_finalize()
{
  chain_.reset();
  reader_.reset();
  return AS_OK;
}

} /* namespace comptonsoft */
