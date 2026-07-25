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

#ifndef COMPTONSOFT_NanoGRAMSLightWaveformStore_hh
#define COMPTONSOFT_NanoGRAMSLightWaveformStore_hh 1

#include <anlnext/BasicModule.hh>

#include <array>
#include <cstdint>
#include <vector>

#include "NanoGRAMSEvent.hh"
#include "NanoGRAMSLightWaveform.hh"

namespace comptonsoft
{

class NanoGRAMSLightWaveformStore : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(NanoGRAMSLightWaveformStore, 1.0);

public:
  NanoGRAMSLightWaveformStore();
  ~NanoGRAMSLightWaveformStore() override;

  anlnext::ANLStatus mod_analyze() override;

  void push(int64_t raw_event_id, int light_ch, grams::LightWaveform waveform);

  bool isValid(int light_ch) const { return valid_[light_ch]; }
  const grams::LightWaveform& waveform(int light_ch) const { return stored_[light_ch]; }
  int64_t currentRawEventId() const { return rawEventId_; }

private:
  std::array<grams::LightWaveform, NUM_CH_DPP_MAX> stored_{};
  std::array<bool, NUM_CH_DPP_MAX> valid_{};
  int64_t rawEventId_ = -1;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightWaveformStore_hh */
