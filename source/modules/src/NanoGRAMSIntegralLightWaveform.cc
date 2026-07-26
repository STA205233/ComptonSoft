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

#include "NanoGRAMSIntegralLightWaveform.hh"

#include "AstroUnits.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "NanoGRAMSLightWaveformStore.hh"

using namespace anlnext;
using namespace anlgeant4::unit;

namespace comptonsoft {
NanoGRAMSIntegralLightWaveform::NanoGRAMSIntegralLightWaveform() = default;
NanoGRAMSIntegralLightWaveform::~NanoGRAMSIntegralLightWaveform() = default;

ANLStatus NanoGRAMSIntegralLightWaveform::mod_define()
{
  VNanoGRAMSLightWaveformQuery::mod_define();
  define_parameter("x_min", &mod_class::xMin_, CLHEP::us, "us");
  define_parameter("x_max", &mod_class::xMax_, CLHEP::us, "us");
  define_parameter("set_to_hit", &mod_class::set_to_hit_);
  return AS_OK;
}

ANLStatus NanoGRAMSIntegralLightWaveform::mod_initialize()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (channels_.empty()) {
    std::cerr << module_id() << ": no light analysis channels configured" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }
  setHistValueName("Integral [mV us]");

  if (set_to_hit_) {
    if (!exist_module("CSHitCollection")) {
      std::cerr << module_id() << ": set_to_hit requires CSHitCollection "
                 << "in the same ANL chain." << std::endl;
      return AS_QUIT_ALL_ERROR;
    }
    get_module_NC("CSHitCollection", &hitCollection_);
  }

  return AS_OK;
}

ANLStatus NanoGRAMSIntegralLightWaveform::mod_analyze()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_analyze();
  resetHistValue();
  if (status != AS_OK) {
    return status;
  }
  if (isSkipLoop()) {
    return AS_OK;
  }

  integrate();

  if (set_to_hit_ && Valid()) {
    applyToHits();
  }

  return AS_OK;
}

void NanoGRAMSIntegralLightWaveform::applyToHits()
{
  const double value = HistValue();
  const int num_time_groups = hitCollection_->NumberOfTimeGroups();
  for (int g = 0; g < num_time_groups; ++g) {
    for (auto& hit : hitCollection_->getHits(g)) {
      hit->setEPI(value);
    }
  }
}

void NanoGRAMSIntegralLightWaveform::integrate()
{
  namespace unit = anlgeant4::unit;
  constexpr double millivolt = unit::volt / 1000.0;
  const double x_min_us = xMin_ / unit::us;
  const double x_max_us = xMax_ / unit::us;

  double total_integral = 0.0;
  bool has_valid_channel = false;

  for (const int channel : channels_) {
    if (!lightWaveformStore_->isValid(channel)) {
      continue;
    }

    const grams::LightWaveform& waveform = lightWaveformStore_->waveform(channel);
    const std::size_t nbins = waveform.values.size();
    if (nbins == 0) {
      continue;
    }

    const double bin_width_us =
        (waveform.xhigh_us - waveform.xlow_us) / static_cast<double>(nbins);
    if (bin_width_us <= 0.0) {
      continue;
    }

    double integral = 0.0;
    for (std::size_t i = 0; i < nbins; ++i) {
      const double center_us = waveform.xlow_us + (static_cast<double>(i) + 0.5) * bin_width_us;
      if (center_us < x_min_us || center_us >= x_max_us) {
        continue;
      }
      const double value_mV = waveform.values[i] / millivolt;
      integral += value_mV * bin_width_us;
    }

    total_integral += integral * cfg_.light_channel_correction[channel];
    has_valid_channel = true;
  }

  if (!has_valid_channel) {
    return;
  }

  setHistValue(total_integral);
}

}
