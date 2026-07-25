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
#include "NanoGRAMSLightWaveformStore.hh"

using namespace anlnext;

namespace comptonsoft {
NanoGRAMSIntegralLightWaveform::NanoGRAMSIntegralLightWaveform() = default;
NanoGRAMSIntegralLightWaveform::~NanoGRAMSIntegralLightWaveform() = default;

ANLStatus NanoGRAMSIntegralLightWaveform::mod_define()
{
  VNanoGRAMSLightWaveformQuery::mod_define();
  define_parameter("x_min", &mod_class::xMin_, CLHEP::us, "us");
  define_parameter("x_max", &mod_class::xMax_, CLHEP::us, "us");
  return AS_OK;
}

ANLStatus NanoGRAMSIntegralLightWaveform::mod_analyze()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_analyze();
  if (status != AS_OK) {
    return status;
  }
  if (isSkipLoop()) {
    return AS_OK;
  }

  integrate();
  return AS_OK;
}

void NanoGRAMSIntegralLightWaveform::integrate()
{
  namespace unit = anlgeant4::unit;
  constexpr double millivolt = unit::volt / 1000.0;
  const double x_min_us = xMin_ / unit::us;
  const double x_max_us = xMax_ / unit::us;

  double integral = 0.0;
  for (const int light_ch : channels_) {
    if (!lightWaveformStore_->isValid(light_ch)) {
      continue;
    }

    const grams::LightWaveform& waveform = lightWaveformStore_->waveform(light_ch);
    const std::size_t nbins = waveform.values.size();
    if (nbins == 0) {
      continue;
    }

    const double bin_width_us =
        (waveform.xhigh_us - waveform.xlow_us) / static_cast<double>(nbins);
    if (bin_width_us <= 0.0) {
      continue;
    }

    for (std::size_t i = 0; i < nbins; ++i) {
      const double center_us = waveform.xlow_us + (static_cast<double>(i) + 0.5) * bin_width_us;
      if (center_us < x_min_us || center_us >= x_max_us) {
        continue;
      }
      const double value_mV = waveform.values[i] / millivolt;
      integral += value_mV * bin_width_us;
    }
  }

  setHistValue(integral);
}

}
