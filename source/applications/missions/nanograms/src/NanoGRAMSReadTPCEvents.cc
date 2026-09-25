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

#include "NanoGRAMSReadTPCEvents.hh"

#include <TFile.h>
#include <TTree.h>

#include "AstroUnits.hh"
#include "GainFunctionCubic.hh"
#include "LightData.hh"
#include "NanoGRAMSConstants.hh"
#include "NanoGRAMSMultiChannelData.hh"
#include "NanoGRAMSTemperatureCorrection.hh"
#include "RealDetectorUnitNanoGRAMS.hh"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>

using namespace anlnext;
namespace unit = anlgeant4::unit;

namespace comptonsoft {

static_assert(RealDetectorUnitNanoGRAMS::NumFECs == NUM_VATA, "NanoGRAMS detector sections must correspond to FECs.");
static_assert(RealDetectorUnitNanoGRAMS::NumChannelsPerFEC == NUM_CH_EACH_VATA,
              "NanoGRAMS section channels must correspond to VATA channels.");

NanoGRAMSReadTPCEvents::NanoGRAMSReadTPCEvents() = default;

NanoGRAMSReadTPCEvents::~NanoGRAMSReadTPCEvents() = default;

ANLStatus NanoGRAMSReadTPCEvents::mod_define()
{
  define_parameter("config_file", &mod_class::config_file_);
  define_parameter("dpp_config_file", &mod_class::dpp_config_file_);
  define_parameter("tpctree_files", &mod_class::tpctree_files_);
  define_parameter("gain_tp_file", &mod_class::gain_tp_file_);
  define_parameter("gain_tp_hash", &mod_class::gain_tp_dict_);
  define_parameter("run_id", &mod_class::run_id_);
  define_map_key("fec", "0");
  add_value_element("gain", &mod_class::gain_tp_value_);
  return AS_OK;
}

ANLStatus NanoGRAMSReadTPCEvents::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (tpctree_files_.empty()) {
    throw std::runtime_error("TPC tree input file path is empty.");
  }

  skipped_error_events_ = 0;
  processed_entries_ = 0;
  expected_tpc_entries_ = 0;
  input_file_index_ = 0;
  current_raw_event_offset_ = 0;
  current_raw_event_id_ = -1;

  // settings common to all the input files
  grams::readConfig(cfg_, config_file_);
  setupCalibration();
  selectLightChannels();

  openNextTPCFile();

  return AS_OK;
}

void NanoGRAMSReadTPCEvents::selectLightChannels()
{
  light_channel_used_.fill(false);
  for (const auto* channels : {&cfg_.general_analysis_channels, &cfg_.pileup_analysis_channels}) {
    for (const int dppChannel : *channels) {
      light_channel_used_.at(dppChannel) = true;
    }
  }
}

bool NanoGRAMSReadTPCEvents::openNextTPCFile()
{
  tpc_tree_reader_.reset();
  input_file_.reset();

  while (input_file_index_ < tpctree_files_.size()) {
    const std::string input_path = tpctree_files_[input_file_index_++];
    if (input_path.empty()) {
      continue;
    }

    // the light data settings (DPP configuration) can be different for each input file
    if (dpp_config_file_.empty()) {
      grams::readDPPConfig(cfg_, input_path);
    }
    else {
      grams::readDPPConfigFile(cfg_, dpp_config_file_);
    }

    auto input_file = std::make_unique<TFile>(input_path.c_str(), "READ");
    if (input_file->IsZombie()) {
      throw std::runtime_error("Failed to open input ROOT file: " + input_path);
    }
    TTree* tpc_tree = dynamic_cast<TTree*>(input_file->Get(grams::kTpcTreeName));
    if (!tpc_tree) {
      throw std::runtime_error(std::format("Missing TTree '{}' in {}", grams::kTpcTreeName, input_path));
    }

    const int64_t entries = static_cast<int64_t>(tpc_tree->GetEntries());
    if (entries == 0) {
      std::cout << "[NanoGRAMSReadTPCEvents] skip empty tpctree: " << input_path << "\n";
      continue;
    }
    input_file_ = std::move(input_file);
    tpc_tree_reader_ = std::make_unique<grams::TPCTreeReader>(tpc_tree);
    setupLightDataLayout();
    current_raw_event_offset_ = processed_entries_;
    expected_tpc_entries_ += entries;
    std::cout << "[NanoGRAMSReadTPCEvents] input file: " << input_path << "\n"
              << "[NanoGRAMSReadTPCEvents] tpctree entries: " << entries << "\n";

    return true;
  }
  return false;
}

void NanoGRAMSReadTPCEvents::setupCalibration()
{
  calibration_config_ = readCalibrationConfig(config_file_);
  const std::filesystem::path gain_info_path =
      resolveCalibrationPath(calibration_config_.config_dir, calibration_config_.energy.gain_info_file);

  // ADC2C: gain functions of the MCDs; ccal2ADC: reference test-pulse ADC for the temperature correction
  std::array<GainMatrix, NUM_VATA> adc2c{};
  auto temperatureCorrection = std::make_shared<NanoGRAMSTemperatureCorrection>(NUM_VATA);
  for (int fec = 0; fec < NUM_VATA; ++fec) {
    adc2c[fec] = loadGainMatrix(gain_info_path, std::format("/FEC{}/ADC2C", fec));
    const GainMatrix ccal2adc = loadGainMatrix(gain_info_path, std::format("/FEC{}/ccal2ADC", fec));
    const double referenceADC = evaluateGainCubic(static_cast<double>(calibration_config_.energy.ccal),
                                                  ccal2adc.at(calibration_config_.energy.tp_channel));
    temperatureCorrection->setReferenceADC(fec, referenceADC);
  }

  if (!gain_tp_file_.empty()) {
    const std::filesystem::path gain_tp_path = resolveCalibrationPath(calibration_config_.config_dir, gain_tp_file_);
    for (const TestPulseGainRow& row : readTestPulseGainTable(gain_tp_path)) {
      for (int fec = 0; fec < NUM_VATA; ++fec) {
        if (std::isfinite(row.fec_gain[fec])) {
          temperatureCorrection->addTestPulseADC(fec, row.time, row.fec_gain[fec]);
        }
      }
    }
    std::cout << "[NanoGRAMSReadTPCEvents] gain_tp_file for temperature correction: " << gain_tp_path << std::endl;
  }
  else if (!gain_tp_dict_.empty()) {
    const std::array<double, NUM_VATA> fixed = fixedTestPulseGainsFromHash(gain_tp_dict_);
    for (int fec = 0; fec < NUM_VATA; ++fec) {
      temperatureCorrection->setFixedTestPulseADC(fec, fixed[fec]);
    }
    std::cout << "[NanoGRAMSReadTPCEvents] gain_tp_hash for temperature correction." << std::endl;
  }
  else {
    std::cout << "[NanoGRAMSReadTPCEvents] WARNING: no gain_tp_file/hash. "
              << "Temperature correction factors are 1." << std::endl;
  }

  setupDetectorParameters(adc2c, temperatureCorrection);
}

void NanoGRAMSReadTPCEvents::setupDetectorParameters(
    const std::array<GainMatrix, NUM_VATA>& adc2c,
    const std::shared_ptr<const NanoGRAMSTemperatureCorrection>& temperatureCorrection)
{
  DetectorSystem* detectorManager = getDetectorManager();
  if (detectorManager == nullptr) {
    return;
  }

  for (auto& detector : detectorManager->getDetectors()) {
    if (!detector->checkType(DetectorType::NanoGRAMS)) {
      continue;
    }
    // always RealDetectorUnitNanoGRAMS
    auto* nanograms = static_cast<RealDetectorUnitNanoGRAMS*>(detector.get());
    nanograms->setMaxDriftTime(calibration_config_.energy.max_time);
    nanograms->setElectricField(calibration_config_.general.efield);
    nanograms->setTemperatureCorrection(temperatureCorrection);

    // cluster selection (the thresholds in the yaml override those in the detector parameters XML;
    // the clustering range is given by the XML). The energies are compared with the charge-equivalent
    // EPI for selection (charge x W_ion).
    nanograms->setClusteringEnergyThreshold(cfg_.core_noise_energy_th);
    nanograms->setClusteringSplitThreshold(cfg_.spread_thr_energy);
    nanograms->setCrossFECMergeDriftTimeTolerance(cfg_.cross_fec_merge_drift_time_tolerance);
    nanograms->setDriftTimeLimit(cfg_.drift_time_max);
    nanograms->setClusterPixelCountRange(cfg_.pix_min, cfg_.pix_max);
    for (const auto& [fec, channels] : cfg_.core_exclude_pix) {
      nanograms->setExcludedCorePixels(fec, channels);
    }

    for (int fec = 0; fec < NUM_VATA; ++fec) {
      NanoGRAMSMultiChannelData* mcd = nanograms->getNanoGRAMSMultiChannelData(fec);
      for (int ch = 0; ch < NUM_CH_EACH_VATA; ++ch) {
        // the gain matrix holds cubic parameters from the highest order: p0*x^3 + p1*x^2 + p2*x + p3
        const GainParamArray& p = adc2c[fec][ch];
        mcd->setGainFunction(ch, std::make_shared<GainFunctionCubic>(p[3], p[2], p[1], p[0]));
      }
      // the threshold is compared with the charge-equivalent selection EPI (charge x W_ion)
      mcd->setHitThresholdEnergy(cfg_.spread_thr_energy);
    }
  }
}

void NanoGRAMSReadTPCEvents::fillEventsIntoDetectors()
{
  DetectorSystem* detectorManager = getDetectorManager();
  if (detectorManager == nullptr) {
    return;
  }

  const grams::TPCTreeBuffer& buffer = tpc_tree_reader_->currentBuffer();
  const int waveformLength = buffer.layout().waveform_len;
  for (auto& detector : detectorManager->getDetectors()) {
    if (!detector->checkType(DetectorType::NanoGRAMS)) {
      continue;
    }
    // always RealDetectorUnitNanoGRAMS
    auto* nanograms = static_cast<RealDetectorUnitNanoGRAMS*>(detector.get());
    nanograms->setUnixTime(current_unix_time_);
    for (int fec = 0; fec < NUM_VATA; ++fec) {
      MultiChannelData* mcd = nanograms->getMultiChannelData(fec);
      for (int ch = 0; ch < NUM_CH_EACH_VATA; ++ch) {
        mcd->setRawADC(ch, buffer.adc[fec * NUM_CH_EACH_VATA + ch]);
        mcd->setDataValid(ch, 1);
      }
      // raw TI counter; the wrap-around is handled by the detector unit
      nanograms->setRawTI(fec, buffer.ti[fec]);
      nanograms->setRawDriftTime(fec, buffer.drift_time[fec]);
      nanograms->setDriftTime(fec, grams::driftTimeFromClock(buffer.drift_time[fec]));
    }

    // light waveforms (index of light data = DPP channel):
    // RawWaveform = digitizer ADC converted into voltage; Waveform is initialized with it and corrected later
    const double adcToVoltage = cfg_.adc2mv * (unit::volt / 1000.0);
    for (int dppChannel = 0; dppChannel < nanograms->NumberOfLightData(); ++dppChannel) {
      const int slot = buffer.waveformSlotForDPPChannel(dppChannel);
      if (slot < 0 || !light_channel_used_[dppChannel]) {
        continue;
      }
      LightData* lightData = nanograms->getLightData(dppChannel);
      std::vector<double>& rawWaveform = lightData->RawWaveform();
      const auto begin = buffer.waveform.begin() + static_cast<std::ptrdiff_t>(slot) * waveformLength;
      std::transform(begin, begin + waveformLength, rawWaveform.begin(),
                     [adcToVoltage](int16_t adc) { return adc * adcToVoltage; });
      lightData->Waveform() = rawWaveform;
    }
  }
}

void NanoGRAMSReadTPCEvents::setupLightDataLayout()
{
  DetectorSystem* detectorManager = getDetectorManager();
  if (detectorManager == nullptr) {
    return;
  }

  // the layout is taken from the first entry of the file (the same as the light timing of TPCTreeReader)
  const grams::TPCTreeBuffer& buffer = tpc_tree_reader_->currentBuffer();
  const int waveformLength = buffer.layout().waveform_len;
  for (auto& detector : detectorManager->getDetectors()) {
    if (!detector->checkType(DetectorType::NanoGRAMS)) {
      continue;
    }
    auto* nanograms = static_cast<RealDetectorUnitNanoGRAMS*>(detector.get());
    std::cout << "[NanoGRAMSReadTPCEvents] light data (detector " << nanograms->getID()
              << ", " << nanograms->NumberOfLightData() << " channels, waveform length " << waveformLength << ")\n";
    for (int dppChannel = 0; dppChannel < nanograms->NumberOfLightData(); ++dppChannel) {
      const bool registered = (buffer.waveformSlotForDPPChannel(dppChannel) >= 0);
      const bool valid = (registered && light_channel_used_[dppChannel]);
      nanograms->getLightData(dppChannel)->setValid(valid);
      std::cout << "  DPP ch " << dppChannel << ": registered=" << registered
                << " used=" << light_channel_used_[dppChannel] << " valid=" << valid;
      if (!valid) {
        std::cout << "\n";
        continue;
      }
      const double timeWidth = static_cast<double>(buffer.wave_compress[dppChannel]) * unit::ns;
      // time = 0 at the trigger
      const double triggerDelay = static_cast<double>(cfg_.light_delay_counts[dppChannel]) * 8.0 * timeWidth;
      nanograms->getLightData(dppChannel)->setLayout(waveformLength, timeWidth, -triggerDelay);
      std::cout << " time_width=" << timeWidth / unit::ns << " ns"
                << " time_start=" << -triggerDelay / unit::us << " us\n";
    }
  }
}

bool NanoGRAMSReadTPCEvents::readNextTPCEvent(int64_t& raw_event_id)
{
  while (true) {
    if (!tpc_tree_reader_ && !openNextTPCFile()) {
      return false;
    }
    if (tpc_tree_reader_->readNextEntry(raw_event_id)) {
      return true;
    }
    tpc_tree_reader_.reset();
    input_file_.reset();
  }
}

ANLStatus NanoGRAMSReadTPCEvents::mod_analyze()
{
  int64_t raw_event_id = 0;
  if (!readNextTPCEvent(raw_event_id)) {
    std::cout << "[NanoGRAMSReadTPCEvents] AS_QUIT after processing " << processed_entries_ << " / "
              << expected_tpc_entries_ << " tpctree entries.\n";
    return AS_QUIT;
  }

  raw_event_id += current_raw_event_offset_;
  current_raw_event_id_ = raw_event_id;
  current_unix_time_ = tpc_tree_reader_->currentUnixTime();
  ++processed_entries_;

  // events with data inconsistency are not used at all
  if (!grams::isTPCDataUsable(tpc_tree_reader_->currentBuffer().error_flags)) {
    ++skipped_error_events_;
    return AS_SKIP;
  }

  fillEventsIntoDetectors();

  return AS_OK;
}

ANLStatus NanoGRAMSReadTPCEvents::mod_end_run()
{
  std::cout << "Skipped events by TPC error flags: " << skipped_error_events_ << "\n";
  std::cout << "Total processed tpctree entries: " << processed_entries_ << " / " << expected_tpc_entries_ << "\n";

  tpc_tree_reader_.reset();
  input_file_.reset();
  return AS_OK;
}

} /* namespace comptonsoft */
