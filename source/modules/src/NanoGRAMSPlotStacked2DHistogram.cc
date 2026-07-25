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

#include "NanoGRAMSPlotStacked2DHistogram.hh"
#include "TH2D.h"
#include "NanoGRAMSLightWaveformStore.hh"
#include <string>
using namespace anlnext;
namespace comptonsoft {
using namespace grams;

constexpr double mV = CLHEP::volt / 1000.0;

NanoGRAMSPlotStacked2DHistogram::NanoGRAMSPlotStacked2DHistogram() = default;
NanoGRAMSPlotStacked2DHistogram::~NanoGRAMSPlotStacked2DHistogram() = default;

ANLStatus NanoGRAMSPlotStacked2DHistogram::mod_define()
{
  define_parameter("num_bins_y", &mod_class::numBinsY_);
  define_parameter("y_min", &mod_class::ymin_, 1.0, "mV");
  define_parameter("y_max", &mod_class::ymax_, 1.0, "mV");
  VNanoGRAMSLightWaveformQuery::mod_define();
  return AS_OK;
}

ANLStatus NanoGRAMSPlotStacked2DHistogram::mod_initialize()
{
  const auto status = VNanoGRAMSLightWaveformQuery::mod_initialize();
  if (status != AS_OK) {
    return status;
  }
  mkdir();
  return AS_OK;
}


ANLStatus NanoGRAMSPlotStacked2DHistogram::mod_analyze()
{
  VNanoGRAMSLightWaveformQuery::mod_analyze();
  if (isSkipLoop()) {
    return AS_OK;
  }
  for (auto i: channels_) {
    const auto waveform = lightWaveformStore_->waveform(i);
    if (!lightWaveformStore_->isValid(i)) {
      continue;
    }
    if (!hists_[i]) {
      std::string name = "Hist2D_" + std::to_string(i);
      chdir();
      hists_[i] = new TH2D(name.c_str(), name.c_str(), waveform.values.size(), waveform.xlow_us, waveform.xhigh_us,
                           numBinsY_, ymin_, ymax_);
    }
    const size_t sz = waveform.values.size();
    for (size_t ibin = 0; ibin < sz; ++ibin) {
      const double x = hists_[i]->GetXaxis()->GetBinCenter(ibin + 1);
      hists_[i]->Fill(x, waveform.values[ibin] / mV);
    }
  }
  return AS_OK;
}
} /* namespace comptonsoft*/