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

#include "NanoGRAMSLightWaveformStage.hh"

#include "NanoGRAMSLightWaveformStore.hh"

#include <cmath>

using namespace anlnext;

namespace comptonsoft
{

VNanoGRAMSLightWaveformStage::VNanoGRAMSLightWaveformStage() = default;

VNanoGRAMSLightWaveformStage::~VNanoGRAMSLightWaveformStage() = default;

ANLStatus VNanoGRAMSLightWaveformStage::mod_analyze()
{
  for (const int light_ch : channels_) {
    if (!lightWaveformStore_->isValid(light_ch)) {
      continue;
    }

    grams::LightWaveform waveform = lightWaveformStore_->waveform(light_ch);
    if (waveform.values.empty()) {
      continue;
    }

    const double bin_width_us =
        (waveform.xhigh_us - waveform.xlow_us) / static_cast<double>(waveform.values.size());
    const int wave_compress = std::max(1, static_cast<int>(std::lround(bin_width_us * 1000.0)));

    applyCorrection(waveform.values, wave_compress);

    lightWaveformStore_->push(lightWaveformStore_->currentRawEventId(), light_ch, waveform);
  }

  return AS_OK;
}

} /* namespace comptonsoft */
