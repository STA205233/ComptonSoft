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

#ifndef COMPTONSOFT_NanoGRAMSConfig_H
#define COMPTONSOFT_NanoGRAMSConfig_H 1

#include <array>
#include <map>
#include <string>
#include <vector>

#include "AstroUnits.hh"
#include "NanoGRAMSConstants.hh"

namespace comptonsoft
{
namespace grams
{

namespace unit = anlgeant4::unit;

enum class LightEventSelectionMode
{
  Disabled,
  GammaRequired,
  VetoOnly,
};

// how the pedestal of the light waveform is estimated
enum class LightPedestalMethod
{
  ValueRange, // mean of the samples whose values are in [pedestal_range_min, pedestal_range_max] (ADC)
  TimeWindow, // mean of the samples in the time window pedestal_time_window (time = 0 at the trigger)
};

// how the waveforms of the channels in an analysis group are combined
enum class LightAnalysisMethod
{
  Average,     // the waveforms are averaged sample by sample, then analyzed
  EachChannel, // each waveform is analyzed, and the maximum over the channels is taken
};

struct Config
{
  int daq_time      = 0;
  int pix_min       = 0;
  int pix_max       = 0;

  double adc2mv           = (1.0 / 8192.0) * 1000.0;
  double core_noise_energy_th = 0.0 * anlgeant4::unit::keV;
  double light_gamma_thr  = 0.0;
  double light_cosmic_thr = 0.0;
  double spread_thr_energy = 0.0 * anlgeant4::unit::keV;
  double drift_time_max   = 0.0 * anlgeant4::unit::us;
  //double late_window      = 0.0;
  //double late_peak_thr    = 0.0;
  double pre_roi_window      = 0.0;
  double post_roi_window     = 0.0;
  double out_roi_peak_thr    = 0.0;
  double light_transimpedance_feedback_resistance_ohm = 3200.0 * unit::ohm;
  double light_output_impedance_ohm = 50.0 * unit::ohm;
  double timebin_ns_override = 0.0;
  double cross_fec_merge_drift_time_tolerance = -1.0 * anlgeant4::unit::us;

  std::vector<int> general_analysis_channels = {4, 6, 5, 7};
  std::vector<int> pileup_analysis_channels  = {4};
  std::array<int, NUM_CH_DPP_MAX> light_delay_counts{};
  LightAnalysisMethod light_analysis_method = LightAnalysisMethod::Average;
  LightEventSelectionMode light_event_selection_mode = LightEventSelectionMode::GammaRequired;
  bool use_light_for_event_selection = true;

  // light waveform corrections (NanoGRAMSCorrectLightWaveform)
  // pedestal: estimated by light_pedestal_method and subtracted
  bool light_pedestal_correction = false;
  LightPedestalMethod light_pedestal_method = LightPedestalMethod::ValueRange;
  double light_pedestal_range_min = -100.0; // ValueRange (in ADC)
  double light_pedestal_range_max = 0.0;    // ValueRange (in ADC)
  double light_pedestal_time_window_start = 0.0; // TimeWindow (time = 0 at the trigger)
  double light_pedestal_time_window_stop = 0.0;  // TimeWindow
  // digitizer offset of the interleaved ADC phases, estimated in the sample index range [start, stop)
  bool light_digitizer_offset_correction = false;
  int light_digitizer_offset_range_start_index = 0;
  int light_digitizer_offset_range_stop_index = 0;
  // FFT band-pass filter; frequencies in GHz (1/ns)
  bool light_fft_filter = false;
  double light_fft_low_frequency = 0.0;
  double light_fft_high_frequency = 0.0;

  std::map<int, std::vector<int>> core_exclude_pix;
};

void readConfig(Config& cfg, const std::string& config_path);
void readDPPConfigFile(Config& cfg, const std::string& config_path);
void readDPPConfig(Config& cfg, const std::string& tpctree_file);

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSConfig_H */
