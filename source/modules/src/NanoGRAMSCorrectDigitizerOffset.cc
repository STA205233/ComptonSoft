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

#include "NanoGRAMSCorrectDigitizerOffset.hh"

#include "NanoGRAMSLightWaveformCorrection.hh"

using namespace anlnext;

namespace comptonsoft
{

NanoGRAMSCorrectDigitizerOffset::NanoGRAMSCorrectDigitizerOffset() = default;

NanoGRAMSCorrectDigitizerOffset::~NanoGRAMSCorrectDigitizerOffset() = default;

ANLStatus NanoGRAMSCorrectDigitizerOffset::mod_define()
{
  const ANLStatus status = VNanoGRAMSLightWaveformStage::mod_define();
  if (status != AS_OK) {
    return status;
  }
  define_parameter("range_start_index", &mod_class::rangeStartIndex_);
  define_parameter("range_stop_index",  &mod_class::rangeStopIndex_);
  return AS_OK;
}

void NanoGRAMSCorrectDigitizerOffset::applyCorrection(std::vector<double>& waveform, int wave_compress)
{
  grams::correctDigitizerOffset(waveform, wave_compress, rangeStartIndex_, rangeStopIndex_);
}

} /* namespace comptonsoft */
