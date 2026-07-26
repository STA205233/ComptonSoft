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

#ifndef COMPTONSOFT_NanoGRAMSPlot2DHistogram_hh
#define COMPTONSOFT_NanoGRAMSPlot2DHistogram_hh 1


#include "NanoGRAMSLightWaveform.hh"
#include "NanoGRAMSLightWaveformQuery.hh"
#include "NanoGRAMSEvent.hh"
#include <array>

class TH2D;

namespace comptonsoft
{


class NanoGRAMSPlotStacked2DHistogram final : public VNanoGRAMSLightWaveformQuery
{
  DEFINE_ANL_MODULE(NanoGRAMSPlotStacked2DHistogram, 1.0);

public:
  NanoGRAMSPlotStacked2DHistogram();
  ~NanoGRAMSPlotStacked2DHistogram();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_finalize() override;
private:
  std::array<TH2D*, NUM_CH_DPP_MAX> hists_;
  double ymin_ = -50.0; // mV
  double ymax_ = 1000.0; // mV
  int numBinsY_ = 1000;

  bool isNecessaryToCreateHist(const grams::LightWaveform &waveform, int channel) const;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightWaveformStore_hh */
