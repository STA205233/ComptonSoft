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

#ifndef COMPTONSOFT_NanoGRAMSQuickLookTreeIO_H
#define COMPTONSOFT_NanoGRAMSQuickLookTreeIO_H 1

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "NanoGRAMSConstants.hh"

class TFile;
class TTree;

namespace comptonsoft
{

class RealDetectorUnitNanoGRAMS;

namespace grams
{

enum class TPCEventType : int16_t
{
  Error  = -1,
  Other  = 0,
  Gamma  = 1,
  Cosmic = 2,
  PileUp = 3,
  TimeUp = 4,
};

/**
 * Writer of the NanoGRAMS quicklook tree, taking the data from the detector unit.
 * - adu_cmn_sub, cmn: raw ADC and the common mode noise (median) of the MCDs
 * - energy_cmn_sub: EPI of the MCDs (charge x W_ion) [keV]
 * - ti, drift_time: raw counters of the DAQ
 * - waveform: corrected light waveforms (LightData::Waveform) of the valid channels [mV]
 * - light_integrated_charge: light charge integrated in ROI of each DPP channel [C]
 *   (RealDetectorUnitNanoGRAMS::LightIntegratedCharge of the general and pileup analysis channels;
 *    0 for the other channels)
 * - hit_*: pixels of the selected clusters (reconstructed hits)
 * @date 2026-09-24 | rewritten to take the data from RealDetectorUnitNanoGRAMS
 */
class QuickLookTreeOutputWriter
{
public:
  QuickLookTreeOutputWriter(const std::string& output_file_path,
                            const RealDetectorUnitNanoGRAMS& detector,
                            bool save_waveforms = true,
                            int flush_entries = 1000);
  ~QuickLookTreeOutputWriter();

  /**
   * @param selected_clusters indices of the reconstructed hits to be written in hit_* branches
   */
  void fillEvent(int64_t raw_event_id,
                 TPCEventType event_type,
                 RealDetectorUnitNanoGRAMS& detector,
                 const std::vector<int>& selected_clusters);
  std::string close();

private:
  void bindBranches();
  void flush();
  void fillChargeMaps(const RealDetectorUnitNanoGRAMS& detector);
  void fillWaveforms(const RealDetectorUnitNanoGRAMS& detector);
  static std::vector<int16_t> validLightChannels(const RealDetectorUnitNanoGRAMS& detector);

  std::filesystem::path  output_path_;
  std::unique_ptr<TFile> file_;
  std::unique_ptr<TTree> quicklook_tree_;
  bool save_waveforms_ = true;
  int flush_entries_ = 1000;
  int waveform_len_ = 0;
  int waveform_num_channels_ = 0;
  std::string adu_leaflist_;
  std::string energy_leaflist_;
  std::string cmn_leaflist_;
  std::string ti_leaflist_;
  std::string drift_leaflist_;
  std::string wave_compress_leaflist_;
  std::string light_integrated_charge_leaflist_;
  std::string waveform_dpp_ch_leaflist_;
  std::string waveform_leaflist_;

  int64_t raw_event_id_ = 0;
  int16_t event_type_   = 0;
  int16_t cmn_method_   = 0;
  int32_t waveform_len_branch_ = 0;
  int32_t waveform_num_channels_branch_ = 0;
  std::vector<float> adu_cmn_sub_;
  std::vector<float> energy_cmn_sub_;
  std::array<float, NUM_VATA> cmn_{};
  std::array<uint32_t, NUM_VATA> ti_{};
  std::array<uint32_t, NUM_VATA> drift_time_{};
  std::array<uint16_t, NUM_CH_DPP_MAX> wave_compress_{};
  std::array<double, NUM_CH_DPP_MAX> light_integrated_charge_{};
  std::vector<int16_t> waveform_dpp_ch_;
  std::vector<float> waveform_;
  std::vector<int16_t> hit_pixel_fec_;
  std::vector<int16_t> hit_pixel_ch_;
  std::vector<float> hit_pixel_adu_;
  std::vector<float> hit_pixel_energy_;
  std::vector<int16_t> hit_pixel_cluster_id_;
  std::vector<int16_t> hit_num_pixels_;
};

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSQuickLookTreeIO_H */
