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

#include "NanoGRAMSWriteLightWaveform.hh"

#include "NanoGRAMSLightWaveformStore.hh"

#include <TCanvas.h>

#include <iostream>

using namespace anlnext;

namespace comptonsoft
{

NanoGRAMSWriteLightWaveform::NanoGRAMSWriteLightWaveform() = default;

NanoGRAMSWriteLightWaveform::~NanoGRAMSWriteLightWaveform() = default;

ANLStatus NanoGRAMSWriteLightWaveform::mod_define()
{
  const ANLStatus status = VNanoGRAMSLightWaveformQuery::mod_define();
  if (status != AS_OK) {
    return status;
  }
  define_parameter("period",           &mod_class::period_);
  define_parameter("max_saved_events", &mod_class::maxSavedEvents_);
  return AS_OK;
}

ANLStatus NanoGRAMSWriteLightWaveform::mod_initialize()
{
  const ANLStatus status = VNanoGRAMSLightWaveformQuery::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  mkdir();

  return AS_OK;
}

ANLStatus NanoGRAMSWriteLightWaveform::mod_analyze()
{
  if (period_ > 1 && eventCounter_ % period_ != 0) {
    ++eventCounter_;
    return AS_OK;
  }

  if (reachedSaveLimit()) {
    if (!saveLimitWarned_) {
      std::cout << "[WARN] NanoGRAMSWriteLightWaveform: reached max_saved_events="
                << maxSavedEvents_
                << "; further events will not be written to avoid an unbounded "
                   "ROOT file key count.\n";
      saveLimitWarned_ = true;
    }
    ++eventCounter_;
    return AS_OK;
  }

  std::array<grams::LightWaveformChannelView, NUM_CH_DPP_MAX> channel_views{};
  for (const int light_ch : channels_) {
    if (!lightWaveformStore_->isValid(light_ch)) {
      continue;
    }
    const grams::LightWaveform& waveform = lightWaveformStore_->waveform(light_ch);
    channel_views[light_ch] = {true, &waveform.values, waveform.xlow_us, waveform.xhigh_us};
  }

  if (plotBuilder_.updatePlot(lightWaveformStore_->currentRawEventId(), channel_views)) {
    chdir();
    TCanvas* canvas = plotBuilder_.canvas();
    const std::string canvas_name =
        "light_waveform_" + std::to_string(lightWaveformStore_->currentRawEventId());
    canvas->SetName(canvas_name.c_str());
    canvas->Write();
    ++savedEventCount_;
  }

  ++eventCounter_;
  return AS_OK;
}

} /* namespace comptonsoft */
