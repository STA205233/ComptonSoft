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

#include "NanoGRAMSApplyLightFFTFilter.hh"

#include "NanoGRAMSLightWaveformCorrection.hh"

using namespace anlnext;

namespace comptonsoft
{

NanoGRAMSApplyLightFFTFilter::NanoGRAMSApplyLightFFTFilter() = default;

NanoGRAMSApplyLightFFTFilter::~NanoGRAMSApplyLightFFTFilter() = default;

ANLStatus NanoGRAMSApplyLightFFTFilter::mod_define()
{
  const ANLStatus status = VNanoGRAMSLightWaveformStage::mod_define();
  if (status != AS_OK) {
    return status;
  }
  define_parameter("low_frequency",  &mod_class::lowFrequency_);
  define_parameter("high_frequency", &mod_class::highFrequency_);
  return AS_OK;
}

void NanoGRAMSApplyLightFFTFilter::applyCorrection(std::vector<double>& waveform, int wave_compress)
{
  grams::applySimpleFFTFilter(waveform, static_cast<double>(wave_compress), lowFrequency_, highFrequency_);
}

} /* namespace comptonsoft */
