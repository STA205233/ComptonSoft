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

#include "NanoGRAMSLightWaveformPlotBuilder.hh"

#include "AstroUnits.hh"

#include <TCanvas.h>
#include <TH1D.h>
#include <TLegend.h>

#include <cmath>
#include <limits>
#include <string>

namespace comptonsoft
{
namespace grams
{

namespace
{

// Same 8-color palette as nanograms-readout's DppWaveGeneratorGroup.
constexpr std::array<int, 8> kChannelColors = {
    kBlack, kRed, kBlue, kGreen, kOrange + 7, kPink + 10, kViolet, kGray};

const double kMillivolt = anlgeant4::unit::volt / 1000.0;

} // namespace

LightWaveformPlotBuilder::LightWaveformPlotBuilder()
    : canvas_(std::make_shared<TCanvas>("light_waveform_canvas", "Light Waveform", 800, 600)),
      legend_(std::make_shared<TLegend>(0.7, 0.7, 0.9, 0.9))
{
}

LightWaveformPlotBuilder::~LightWaveformPlotBuilder() = default;

bool LightWaveformPlotBuilder::updatePlot(
    int64_t raw_event_id,
    const std::array<LightWaveformChannelView, NUM_CH_DPP_MAX>& channels)
{
  std::vector<TH1D*> drawn_hists;
  double y_min = std::numeric_limits<double>::infinity();
  double y_max = -std::numeric_limits<double>::infinity();

  for (int light_ch = 0; light_ch < NUM_CH_DPP_MAX; ++light_ch) {
    const LightWaveformChannelView& channel = channels[light_ch];
    if (!channel.valid || !channel.values || channel.values->empty()) {
      continue;
    }
    const std::vector<double>& values = *channel.values;
    const int nbins = static_cast<int>(values.size());
    const double xlow = channel.xlow_us;
    const double xhigh = channel.xhigh_us;

    std::shared_ptr<TH1D>& hist = hists_[light_ch];
    constexpr double kAxisTolerance = 1e-9;
    const bool axis_stale =
        hist && (hist->GetNbinsX() != nbins ||
                std::abs(hist->GetXaxis()->GetXmin() - xlow) > kAxisTolerance ||
                std::abs(hist->GetXaxis()->GetXmax() - xhigh) > kAxisTolerance);
    if (!hist || axis_stale) {
      const std::string name = "light_waveform_ch" + std::to_string(light_ch);
      hist = std::make_shared<TH1D>(name.c_str(), name.c_str(), nbins, xlow, xhigh);
      hist->SetDirectory(nullptr);
      hist->SetXTitle("Time [us]");
      hist->SetYTitle("Amplitude [mV]");
      hist->SetLineColor(kChannelColors[light_ch % kChannelColors.size()]);
      hist->SetMarkerColor(kChannelColors[light_ch % kChannelColors.size()]);
    }
    if (!legendRegistered_[light_ch]) {
      const std::string legend_label = "Ch" + std::to_string(light_ch);
      legend_->AddEntry(hist.get(), legend_label.c_str(), "l");
      legendRegistered_[light_ch] = true;
    }

    hist->Reset();
    for (int i = 0; i < nbins; ++i) {
      const double value_mV = values[i] / kMillivolt;
      hist->SetBinContent(i + 1, value_mV);
      hist->SetBinError(i + 1, 0);
      y_min = std::min(y_min, value_mV);
      y_max = std::max(y_max, value_mV);
    }

    drawn_hists.push_back(hist.get());
  }

  if (drawn_hists.empty()) {
    return false;
  }

  const double margin = 0.05 * (y_max - y_min);
  for (TH1D* hist : drawn_hists) {
    hist->SetMinimum(y_min - margin);
    hist->SetMaximum(y_max + margin);
  }

  canvas_->cd();
  canvas_->Clear();
  for (std::size_t i = 0; i < drawn_hists.size(); ++i) {
    drawn_hists[i]->Draw(i == 0 ? "hist" : "hist same");
  }
  legend_->Draw();

  const std::string canvas_name = "light_waveform_" + std::to_string(raw_event_id);
  canvas_->SetName(canvas_name.c_str());

  return true;
}

} /* namespace grams */
} /* namespace comptonsoft */
