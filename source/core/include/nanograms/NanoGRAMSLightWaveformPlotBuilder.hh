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

#ifndef COMPTONSOFT_NanoGRAMSLightWaveformPlotBuilder_H
#define COMPTONSOFT_NanoGRAMSLightWaveformPlotBuilder_H 1

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "NanoGRAMSEvent.hh"

class TCanvas;
class TH1D;
class TLegend;

namespace comptonsoft
{
namespace grams
{

struct LightWaveformChannelView
{
  bool valid = false;
  const std::vector<double>* values = nullptr;
  double xlow_us = 0.0;
  double xhigh_us = 0.0;
};

class LightWaveformPlotBuilder
{
public:
  LightWaveformPlotBuilder();
  ~LightWaveformPlotBuilder();

  bool updatePlot(int64_t raw_event_id,
                  const std::array<LightWaveformChannelView, NUM_CH_DPP_MAX>& channels);

  TCanvas* canvas() const { return canvas_.get(); }

private:
  std::shared_ptr<TCanvas> canvas_;
  std::shared_ptr<TLegend> legend_;
  std::array<std::shared_ptr<TH1D>, NUM_CH_DPP_MAX> hists_{};
  std::array<bool, NUM_CH_DPP_MAX> legendRegistered_{};
};

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightWaveformPlotBuilder_H */
