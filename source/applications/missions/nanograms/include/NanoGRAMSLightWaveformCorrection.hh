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


#ifndef COMPTONSOFT_NanoGRAMSLightWaveformCorrection_H
#define COMPTONSOFT_NanoGRAMSLightWaveformCorrection_H 1

#include <array>
#include <vector>

namespace comptonsoft
{
namespace grams
{

struct PedestalCorrectionResult
{
  double pedestal = 0.0;
  double stddev = 0.0;
};

PedestalCorrectionResult correctPedestal(std::vector<double>& waveform,
                                         double range_min,
                                         double range_max);

constexpr int kNumInterleavedADCPhases = 4;

struct DigitizerOffsetCorrectionResult
{
  std::array<double, kNumInterleavedADCPhases> offset{};
};

DigitizerOffsetCorrectionResult correctDigitizerOffset(std::vector<double>& waveform,
                                                       int wave_compress,
                                                       int range_start_index,
                                                       int range_stop_index);

void applySimpleFFTFilter(std::vector<double>& waveform,
                          double sampling_interval,
                          double low_frequency,
                          double high_frequency);

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightWaveformCorrection_H */
