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
 *************************************************************************/

#include "NanoGRAMSQuickLookTreeIO.hh"

#include <TFile.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

#include "AstroUnits.hh"
#include "DetectorHit.hh"
#include "LightData.hh"
#include "NanoGRAMSMultiChannelData.hh"
#include "RealDetectorUnitNanoGRAMS.hh"

namespace comptonsoft
{
namespace grams
{

namespace
{

namespace unit = anlgeant4::unit;

std::filesystem::path prepareOutputPath(const std::string& output_file_path)
{
  std::filesystem::path output_path(output_file_path);
  const auto output_parent = output_path.parent_path();
  if (!output_parent.empty()) {
    std::filesystem::create_directories(output_parent);
  }
  return output_path;
}

// raw ADC - common mode noise (median), i.e. PHA before the temperature correction
double aduCMNSubtracted(const NanoGRAMSMultiChannelData& mcd, int ch)
{
  return static_cast<double>(mcd.getRawADC(ch)) - mcd.getCommonModeNoise();
}

} /* namespace */

std::vector<int16_t> QuickLookTreeOutputWriter::validLightChannels(const RealDetectorUnitNanoGRAMS& detector)
{
  std::vector<int16_t> channels;
  for (int dpp_ch = 0; dpp_ch < detector.NumberOfLightData(); ++dpp_ch) {
    if (detector.getLightData(dpp_ch)->isValid()) {
      channels.push_back(static_cast<int16_t>(dpp_ch));
    }
  }
  return channels;
}

QuickLookTreeOutputWriter::QuickLookTreeOutputWriter(
    const std::string& output_file_path,
    const RealDetectorUnitNanoGRAMS& detector,
    bool save_waveforms,
    int flush_entries)
    : output_path_(prepareOutputPath(output_file_path)),
      file_(std::make_unique<TFile>(output_path_.string().c_str(), "RECREATE")),
      quicklook_tree_(std::make_unique<TTree>(kQuickLookTreeName, kQuickLookTreeName)),
      save_waveforms_(save_waveforms),
      flush_entries_(std::max(1, flush_entries))
{
  if (file_->IsZombie()) {
    throw std::runtime_error("Failed to create quicklook ROOT file: " +
                             output_path_.string());
  }

  if (save_waveforms_) {
    waveform_dpp_ch_ = validLightChannels(detector);
    if (!waveform_dpp_ch_.empty()) {
      waveform_len_ = detector.getLightData(waveform_dpp_ch_.front())->NumberOfPoints();
    }
  }
  waveform_num_channels_ = static_cast<int>(waveform_dpp_ch_.size());
  waveform_len_branch_ = waveform_len_;
  waveform_num_channels_branch_ = waveform_num_channels_;
  adu_cmn_sub_.assign(NUM_VATA * NUM_CH_EACH_VATA, 0.0f);
  energy_cmn_sub_.assign(NUM_VATA * NUM_CH_EACH_VATA, 0.0f);
  if (save_waveforms_) {
    waveform_.assign(waveform_num_channels_ * waveform_len_, 0.0f);
  }
  quicklook_tree_->SetDirectory(file_.get());
  quicklook_tree_->SetAutoFlush(-flush_entries_);
  quicklook_tree_->SetAutoSave(-flush_entries_);
  bindBranches();
}

QuickLookTreeOutputWriter::~QuickLookTreeOutputWriter() = default;

void QuickLookTreeOutputWriter::fillEvent(int64_t raw_event_id,
                                          TPCEventType event_type,
                                          RealDetectorUnitNanoGRAMS& detector,
                                          const std::vector<int>& selected_clusters)
{
  raw_event_id_ = raw_event_id;
  event_type_   = static_cast<int16_t>(event_type);
  cmn_method_ = (event_type == TPCEventType::Cosmic) ? 1 : 0;

  for (int fec = 0; fec < NUM_VATA; ++fec) {
    ti_[fec]         = static_cast<uint32_t>(std::max<int64_t>(0, detector.RawTI(fec)));
    drift_time_[fec] = static_cast<uint32_t>(std::max<int64_t>(0, detector.RawDriftTime(fec)));
  }

  for (int dpp_ch = 0; dpp_ch < NUM_CH_DPP_MAX; ++dpp_ch) {
    light_integrated_charge_[dpp_ch] =
        (dpp_ch < detector.NumberOfLightData()) ? detector.LightIntegratedCharge(dpp_ch) / unit::coulomb : 0.0;
    wave_compress_[dpp_ch] = 0;
    if (dpp_ch < detector.NumberOfLightData() && detector.getLightData(dpp_ch)->isValid()) {
      wave_compress_[dpp_ch] =
          static_cast<uint16_t>(std::lround(detector.getLightData(dpp_ch)->TimeWidth() / unit::ns));
    }
  }
  if (save_waveforms_) {
    fillWaveforms(detector);
  }
  fillChargeMaps(detector);

  hit_pixel_fec_.clear();
  hit_pixel_ch_.clear();
  hit_pixel_adu_.clear();
  hit_pixel_energy_.clear();
  hit_pixel_cluster_id_.clear();
  hit_num_pixels_.clear();
  const std::vector<std::vector<int>>& correspondence = detector.ClusterCorrespondence();
  for (std::size_t icluster = 0; icluster < selected_clusters.size(); ++icluster) {
    const std::vector<int>& pixels = correspondence.at(selected_clusters[icluster]);
    hit_num_pixels_.push_back(static_cast<int16_t>(pixels.size()));
    for (const int pixel_index : pixels) {
      const DetectorHit_sptr pixel = detector.getDetectorHit(pixel_index);
      const int fec = pixel->DetectorSection();
      const int ch = pixel->DetectorChannel();
      hit_pixel_fec_.push_back(static_cast<int16_t>(fec));
      hit_pixel_ch_.push_back(static_cast<int16_t>(ch));
      hit_pixel_adu_.push_back(static_cast<float>(aduCMNSubtracted(*detector.getNanoGRAMSMultiChannelData(fec), ch)));
      hit_pixel_energy_.push_back(static_cast<float>(pixel->EPI() / unit::keV));
      hit_pixel_cluster_id_.push_back(static_cast<int16_t>(icluster));
    }
  }

  quicklook_tree_->Fill();
  if (quicklook_tree_->GetEntries() % flush_entries_ == 0) {
    flush();
  }
}

void QuickLookTreeOutputWriter::flush()
{
  file_->cd();
  quicklook_tree_->FlushBaskets();
  quicklook_tree_->AutoSave();
  file_->Flush();
}

std::string QuickLookTreeOutputWriter::close()
{
  file_->cd();
  quicklook_tree_->Write();
  file_->Write();
  const auto entries = quicklook_tree_->GetEntries();
  quicklook_tree_->SetDirectory(nullptr);
  file_->Close();

  std::cout << "[ROOT] Saved quicklook file: " << output_path_.string()
            << " (entries=" << entries << ")\n";
  return output_path_.string();
}

void QuickLookTreeOutputWriter::bindBranches()
{
  adu_leaflist_             = std::format("adu_cmn_sub[{}][{}]/F", NUM_VATA, NUM_CH_EACH_VATA);
  energy_leaflist_          = std::format("energy_cmn_sub[{}][{}]/F", NUM_VATA, NUM_CH_EACH_VATA);
  cmn_leaflist_             = std::format("cmn[{}]/F", NUM_VATA);
  ti_leaflist_              = std::format("ti[{}]/i", NUM_VATA);
  drift_leaflist_           = std::format("drift_time[{}]/i", NUM_VATA);
  wave_compress_leaflist_   = std::format("wave_compress[{}]/s", NUM_CH_DPP_MAX);
  light_integrated_charge_leaflist_ = std::format("light_integrated_charge[{}]/D", NUM_CH_DPP_MAX);
  quicklook_tree_->Branch("raw_event_id", &raw_event_id_, "raw_event_id/L");
  quicklook_tree_->Branch("event_type",   &event_type_,   "event_type/S");
  quicklook_tree_->Branch("cmn_method",   &cmn_method_,   "cmn_method/S");
  quicklook_tree_->Branch("adu_cmn_sub",  adu_cmn_sub_.data(),  adu_leaflist_.c_str());
  quicklook_tree_->Branch("energy_cmn_sub",
                          energy_cmn_sub_.data(),
                          energy_leaflist_.c_str());
  quicklook_tree_->Branch("cmn",          cmn_.data(),          cmn_leaflist_.c_str());
  quicklook_tree_->Branch("ti",           ti_.data(),           ti_leaflist_.c_str());
  quicklook_tree_->Branch("drift_time",   drift_time_.data(),   drift_leaflist_.c_str());
  quicklook_tree_->Branch("wave_compress", wave_compress_.data(),
                          wave_compress_leaflist_.c_str());
  quicklook_tree_->Branch("light_integrated_charge", light_integrated_charge_.data(),
                          light_integrated_charge_leaflist_.c_str());
  if (save_waveforms_) {
    waveform_dpp_ch_leaflist_ = std::format("waveform_dpp_ch[{}]/S", waveform_num_channels_);
    waveform_leaflist_        = std::format("waveform[{}][{}]/F", waveform_num_channels_, waveform_len_);
    quicklook_tree_->Branch("waveform_len", &waveform_len_branch_, "waveform_len/I");
    quicklook_tree_->Branch("waveform_num_channels",
                            &waveform_num_channels_branch_,
                            "waveform_num_channels/I");
    quicklook_tree_->Branch("waveform_dpp_ch",
                            waveform_dpp_ch_.data(),
                            waveform_dpp_ch_leaflist_.c_str());
    quicklook_tree_->Branch("waveform",     waveform_.data(),     waveform_leaflist_.c_str());
  }
  quicklook_tree_->Branch("hit_pixel_fec",        &hit_pixel_fec_);
  quicklook_tree_->Branch("hit_pixel_ch",         &hit_pixel_ch_);
  quicklook_tree_->Branch("hit_pixel_adu",        &hit_pixel_adu_);
  quicklook_tree_->Branch("hit_pixel_energy",     &hit_pixel_energy_);
  quicklook_tree_->Branch("hit_pixel_cluster_id", &hit_pixel_cluster_id_);
  quicklook_tree_->Branch("hit_num_pixels",       &hit_num_pixels_);
}

void QuickLookTreeOutputWriter::fillChargeMaps(const RealDetectorUnitNanoGRAMS& detector)
{
  for (int fec = 0; fec < NUM_VATA; ++fec) {
    const NanoGRAMSMultiChannelData& mcd = *detector.getNanoGRAMSMultiChannelData(fec);
    cmn_[fec] = static_cast<float>(mcd.getCommonModeNoise());
    for (int ch = 0; ch < NUM_CH_EACH_VATA; ++ch) {
      const int index = fec * NUM_CH_EACH_VATA + ch;
      adu_cmn_sub_[index] = static_cast<float>(aduCMNSubtracted(mcd, ch));
      energy_cmn_sub_[index] = static_cast<float>(mcd.getEPI(ch) / unit::keV);
    }
  }
}

void QuickLookTreeOutputWriter::fillWaveforms(const RealDetectorUnitNanoGRAMS& detector)
{
  if (validLightChannels(detector) != waveform_dpp_ch_) {
    throw std::runtime_error("Valid light channels changed; the quicklook waveform layout is fixed by the first event.");
  }

  for (std::size_t output_slot = 0; output_slot < waveform_dpp_ch_.size(); ++output_slot) {
    const std::vector<double>& waveform = detector.getLightData(waveform_dpp_ch_[output_slot])->Waveform();
    if (static_cast<int>(waveform.size()) != waveform_len_) {
      throw std::runtime_error("Light waveform length changed; the quicklook waveform layout is fixed by the first event.");
    }
    const std::size_t output_offset = output_slot * waveform_len_;
    for (int i = 0; i < waveform_len_; ++i) {
      waveform_[output_offset + i] = static_cast<float>(waveform[i] / (unit::volt / 1000.0));
    }
  }
}

} /* namespace grams */
} /* namespace comptonsoft */
