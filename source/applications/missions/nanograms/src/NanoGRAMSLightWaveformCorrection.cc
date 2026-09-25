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

#include "NanoGRAMSLightWaveformCorrection.hh"

#include <algorithm>
#include <cmath>

#include "NanoGRAMSSimpleFilter.hh"
#include "TH1D.h"

namespace comptonsoft
{
namespace grams
{

PedestalCorrectionResult correctPedestal(std::vector<double>& waveform, double range_min, double range_max)
{
  PedestalCorrectionResult result;
  double sum = 0.0;
  double sum_sq = 0.0;
  int count = 0;
  for (const double sample : waveform) {
    if (sample >= range_min && sample <= range_max) {
      sum += sample;
      sum_sq += sample * sample;
      ++count;
    }
  }

  if (count > 0) {
    result.pedestal = sum / count;
    const double variance = sum_sq / count - result.pedestal * result.pedestal;
    result.stddev = variance > 0.0 ? std::sqrt(variance) : 0.0;
  }

  for (double& sample : waveform) {
    sample -= result.pedestal;
  }
  return result;
}

DigitizerOffsetCorrectionResult correctDigitizerOffset(std::vector<double>& waveform, int wave_compress, int range_start_index, int range_stop_index)
{
  DigitizerOffsetCorrectionResult result;
  const int groups = std::max(1, kNumInterleavedADCPhases / std::max(1, wave_compress));
  std::array<int, kNumInterleavedADCPhases> count{};

  const int size = static_cast<int>(waveform.size());
  const int start = std::max(0, range_start_index);
  const int stop = std::min(size, range_stop_index);
  for (int idx = start; idx < stop; ++idx) {
    const int phase = idx % groups;
    result.offset[phase] += waveform[idx];
    ++count[phase];
  }

  for (int phase = 0; phase < groups; ++phase) {
    result.offset[phase] = (count[phase] > 0) ? result.offset[phase] / count[phase] : 0.0;
  }

  for (int idx = 0; idx < size; ++idx) {
    waveform[idx] -= result.offset[idx % groups];
  }
  return result;
}

void applySimpleFFTFilter(std::vector<double>& waveform, double sampling_interval, double low_frequency, double high_frequency)
{
  const int n = static_cast<int>(waveform.size());
  if (n == 0) {
    return;
  }

  auto input_hist = std::make_shared<TH1D>("NanoGRAMSSimpleFFTFilterInput",
                                           "NanoGRAMSSimpleFFTFilterInput",
                                           n, 0.0, n * sampling_interval);
  input_hist->SetDirectory(nullptr);
  for (int i = 0; i < n; ++i) {
    input_hist->SetBinContent(i + 1, waveform[i]);
  }

  SimpleFilterParam param;
  param.lowFrequency = low_frequency;
  param.highFrequency = high_frequency;
  SimpleFilter filter;
  filter.SetParam(param);
  const std::shared_ptr<TH1D> filtered_hist = filter.Exec(input_hist);
  if (!filtered_hist) {
    return;
  }

  for (int i = 0; i < n; ++i) {
    waveform[i] = filtered_hist->GetBinContent(i + 1);
  }
}

} /* namespace grams */
} /* namespace comptonsoft */
