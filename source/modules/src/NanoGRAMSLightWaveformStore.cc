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

#include "NanoGRAMSLightWaveformStore.hh"

using namespace anlnext;

namespace comptonsoft
{
using namespace grams;

NanoGRAMSLightWaveformStore::NanoGRAMSLightWaveformStore() = default;

NanoGRAMSLightWaveformStore::~NanoGRAMSLightWaveformStore() = default;

ANLStatus NanoGRAMSLightWaveformStore::mod_analyze()
{
  valid_.fill(false);
  rawEventId_ = -1;
  return AS_OK;
}

void NanoGRAMSLightWaveformStore::push(int64_t raw_event_id, int light_ch, LightWaveform waveform)
{
  rawEventId_ = raw_event_id;
  stored_[light_ch] = std::move(waveform);
  valid_[light_ch] = true;
}

} /* namespace comptonsoft */
