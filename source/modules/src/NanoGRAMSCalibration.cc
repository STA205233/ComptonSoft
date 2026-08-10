/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                    *
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

#include "NanoGRAMSCalibration.hh"

#include "DetectorHit.hh"
#include "NanoGRAMSHitCalibrator.hh"
#include "NanoGRAMSHitExtraction.hh"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace anlnext;

namespace comptonsoft {

namespace fs = std::filesystem;

namespace {

int64_t gainTimeBin(double unix_time, double cache_seconds)
{
  if (cache_seconds > 0.0) {
    return static_cast<int64_t>(std::floor(unix_time / cache_seconds));
  }
  return static_cast<int64_t>(unix_time);
}

} // namespace

NanoGRAMSCalibration::NanoGRAMSCalibration() = default;

NanoGRAMSCalibration::~NanoGRAMSCalibration() = default;

ANLStatus NanoGRAMSCalibration::mod_define()
{
  define_parameter("gain_tp_file", &mod_class::gain_tp_file_);
  define_parameter("gain_tp_hash", &mod_class::gain_tp_dict_);
  define_parameter("gain_cache_seconds", &mod_class::gain_cache_seconds_);
  define_map_key("fec", "0");
  add_value_element("gain", &mod_class::gain_tp_value_);
  return AS_OK;
}

ANLStatus NanoGRAMSCalibration::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (!exist_module("NanoGRAMSHitExtraction")) {
    throw std::runtime_error(
        "NanoGRAMSCalibration requires NanoGRAMSHitExtraction in the same ANL chain.");
  }
  get_module("NanoGRAMSHitExtraction", &data_reduction_);

  calibration_config_ = readCalibrationConfig(data_reduction_->configFilePath());

  if (!gain_tp_file_.empty()) {
    const fs::path gain_tp_path =
        resolveCalibrationPath(calibration_config_.config_dir, gain_tp_file_);
    gain_tp_table_ = readTestPulseGainTable(gain_tp_path);
    use_event_time_gain_ = true;
    std::cout << "gain_tp_file: " << gain_tp_path << std::endl;
    std::cout << "TPC event unixtime is used for gain interpolation."
              << std::endl;
    std::cout << "gain_cache_seconds: "
              << gain_cache_seconds_ << std::endl;
  }
  else {
    if (gain_tp_dict_.empty()) {
      throw std::runtime_error(
          "NanoGRAMSCalibration needs either gain_tp_file or gain_tp_hash.");
    }
    calibration_config_.energy.tp_adc_values =
        fixedTestPulseGainsFromHash(gain_tp_dict_);
    std::cout << "gain_tp_hash: using fixed FEC gain values." << std::endl;
  }

  const fs::path gain_info_path =
      resolveCalibrationPath(calibration_config_.config_dir,
                             calibration_config_.energy.gain_info_file);
  const fs::path spline_path =
      resolveCalibrationPath(calibration_config_.config_dir,
                             calibration_config_.energy.q_to_kev_spline_file);

  tpc_property_.loadParamCoulomb2keVForSpline3D(spline_path,
                                                calibration_config_.general.efield);
  tpc_property_.loadParamGainMatrices(gain_info_path);
  tpc_property_.setDriftVelocity(electronDriftVelocity(
      calibration_config_.general.temperature,
      calibration_config_.general.efield));
  tpc_property_.setAnodePosZ(calibration_config_.position.anode_pos_z);
  if (!use_event_time_gain_) {
    tpc_property_.applyTemperatureCorrection(
        calibration_config_.energy.tp_channel,
        calibration_config_.energy.ccal,
        calibration_config_.energy.tp_adc_values);
    std::cout << "tp_adc_values [ ";
    for (std::size_t i = 0; i < calibration_config_.energy.tp_adc_values.size(); ++i) {
      if (i != 0) {
        std::cout << ", ";
      }
      std::cout << calibration_config_.energy.tp_adc_values[i];
    }
    std::cout << " ]" << std::endl;
  }

  if (!exist_module("CSHitCollection")) {
    throw std::runtime_error(
        "NanoGRAMSCalibration requires CSHitCollection in the same ANL chain.");
  }
  get_module_NC("CSHitCollection", &hitCollection_);

  define_evs("NanoGRAMSHitTree:Fill");
  return AS_OK;
}

void NanoGRAMSCalibration::updateGainCorrectionForCurrentEvent()
{
  if (!use_event_time_gain_) {
    return;
  }

  const uint32_t unix_time = data_reduction_->currentUnixTime();
  if (unix_time == 0) {
    throw std::runtime_error(
        "TPC tree unixtime is zero; cannot apply event-time gain correction.");
  }

  const int64_t time_bin = gainTimeBin(
      static_cast<double>(unix_time),
      gain_cache_seconds_);
  if (time_bin == cached_gain_time_bin_) {
    return;
  }

  calibration_config_.energy.tp_adc_values =
      interpolatedTestPulseGains(gain_tp_table_, static_cast<double>(unix_time));
  tpc_property_.applyTemperatureCorrection(
      calibration_config_.energy.tp_channel,
      calibration_config_.energy.ccal,
      calibration_config_.energy.tp_adc_values);
  cached_gain_time_bin_ = time_bin;
}

ANLStatus NanoGRAMSCalibration::mod_analyze()
{
  if (!data_reduction_ || !data_reduction_->hasCurrentEvent()) {
    return AS_OK;
  }

  updateGainCorrectionForCurrentEvent();

  const auto hits =
      buildCalibratedHits(calibration_config_,
                          tpc_property_,
                          data_reduction_->currentEventHits());
  if (hits.empty()) {
    return AS_OK;
  }

  const int32_t run_id = data_reduction_->runId();
  const int32_t event_id = static_cast<int32_t>(data_reduction_->currentEventId());
  for (auto& hit : hits) {
    hit->setRunID(run_id);
    hit->setEventID(event_id);
    hitCollection_->insertHit(hit);
  }
  set_evs("NanoGRAMSHitTree:Fill");
  return AS_OK;
}

ANLStatus NanoGRAMSCalibration::mod_end_run()
{
  data_reduction_ = nullptr;
  return AS_OK;
}

} /* namespace comptonsoft */
