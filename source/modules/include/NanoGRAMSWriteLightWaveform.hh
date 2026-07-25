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

#ifndef COMPTONSOFT_NanoGRAMSWriteLightWaveform_hh
#define COMPTONSOFT_NanoGRAMSWriteLightWaveform_hh 1

#include <cstdint>

#include "NanoGRAMSLightWaveformPlotBuilder.hh" // grams::LightWaveformPlotBuilder
#include "NanoGRAMSLightWaveformQuery.hh"

namespace comptonsoft
{

class NanoGRAMSWriteLightWaveform : public VNanoGRAMSLightWaveformQuery
{
  DEFINE_ANL_MODULE(NanoGRAMSWriteLightWaveform, 1.0);

public:
  NanoGRAMSWriteLightWaveform();
  ~NanoGRAMSWriteLightWaveform() override;

  anlnext::ANLStatus mod_define()     override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze()    override;

private:
  bool reachedSaveLimit() const {
    return maxSavedEvents_ > 0 && savedEventCount_ >= maxSavedEvents_;
  }

  int period_ = 1;
  int64_t maxSavedEvents_ = 10000;

  grams::LightWaveformPlotBuilder plotBuilder_;
  int64_t eventCounter_ = 0;
  int64_t savedEventCount_ = 0;
  bool saveLimitWarned_ = false;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSWriteLightWaveform_hh */
