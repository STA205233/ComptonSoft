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

#include "NanoGRAMSAnalyzeLight.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "AstroUnits.hh"
#include "FlagDefinition.hh"
#include "LightData.hh"
#include "NanoGRAMSConfig.hh"
#include "NanoGRAMSReadTPCEvents.hh"
#include "RealDetectorUnitNanoGRAMS.hh"

using namespace anlnext;
namespace unit = anlgeant4::unit;

namespace comptonsoft {

namespace {

constexpr double DigitizerInputImpedance = 50.0 * unit::ohm;

} // namespace

NanoGRAMSAnalyzeLight::NanoGRAMSAnalyzeLight() = default;

NanoGRAMSAnalyzeLight::~NanoGRAMSAnalyzeLight() = default;

ANLStatus NanoGRAMSAnalyzeLight::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  get_module("NanoGRAMSReadTPCEvents", &reader_);
  config_ = &reader_->config();
  return AS_OK;
}

ANLStatus NanoGRAMSAnalyzeLight::mod_analyze()
{
  const grams::Config& cfg = *config_;
  for (auto& detector : getDetectorManager()->getDetectors()) {
    if (!detector->checkType(DetectorType::NanoGRAMS)) {
      continue;
    }
    // always RealDetectorUnitNanoGRAMS
    // Waveform (voltage) is given by the reader and may be corrected by NanoGRAMSCorrectLightWaveform
    auto* nanograms = static_cast<RealDetectorUnitNanoGRAMS*>(detector.get());

    const std::vector<int> generalChannels = collectValidChannels(*nanograms, cfg.general_analysis_channels);
    if (!generalChannels.empty()) {
      const LightPeaks peaks = analyzeChannelGroup(*nanograms, generalChannels);
      if (peaks.ROI > cfg.light_gamma_thr) {
        nanograms->addEventFlags(nanograms_event_flag::LightGamma);
      }
      if (peaks.ROI > cfg.light_cosmic_thr) {
        nanograms->addEventFlags(nanograms_event_flag::LightCosmic);
      }
    }
    // integrated charges; their sum over the general analysis channels is the photon count of the event
    double photonCount = 0.0;
    for (const int dppChannel : generalChannels) {
      const double charge = integratedROICharge(*nanograms->getLightData(dppChannel));
      nanograms->setLightIntegratedCharge(dppChannel, charge);
      if (std::isfinite(charge)) {
        photonCount += charge;
      }
    }
    nanograms->setPhotonCount(photonCount);

    const std::vector<int> pileupChannels = collectValidChannels(*nanograms, cfg.pileup_analysis_channels);
    if (!pileupChannels.empty()) {
      const LightPeaks peaks = analyzeChannelGroup(*nanograms, pileupChannels);
      if (peaks.preROI > cfg.out_roi_peak_thr || peaks.postROI > cfg.out_roi_peak_thr) {
        nanograms->addEventFlags(nanograms_event_flag::LightPileup);
      }
    }
    // integrated charges of the pileup analysis channels (not included in the photon count)
    for (const int dppChannel : pileupChannels) {
      if (std::find(generalChannels.begin(), generalChannels.end(), dppChannel) == generalChannels.end()) {
        nanograms->setLightIntegratedCharge(dppChannel, integratedROICharge(*nanograms->getLightData(dppChannel)));
      }
    }

    if (numLoggedEvents_ < NumEventsToLog) {
      printEventSummary(*nanograms, generalChannels, pileupChannels);
    }
  }

  ++numLoggedEvents_;
  return AS_OK;
}

void NanoGRAMSAnalyzeLight::printEventSummary(const RealDetectorUnitNanoGRAMS& detector,
                                              const std::vector<int>& generalChannels,
                                              const std::vector<int>& pileupChannels) const
{
  auto printChannels = [](const std::vector<int>& channels) {
    std::cout << "[";
    for (std::size_t i = 0; i < channels.size(); ++i) {
      std::cout << (i == 0 ? "" : ", ") << channels[i];
    }
    std::cout << "]";
  };

  std::cout << "[NanoGRAMSAnalyzeLight] event " << numLoggedEvents_ << " (detector " << detector.getID() << ")\n"
            << "  valid general channels: ";
  printChannels(generalChannels);
  std::cout << "\n  valid pileup channels: ";
  printChannels(pileupChannels);
  std::cout << "\n";
  if (!generalChannels.empty()) {
    const LightPeaks peaks = analyzeChannelGroup(detector, generalChannels);
    std::cout << "  general peak in ROI: " << peaks.ROI / (unit::volt / 1000.0) << " mV\n";
  }
  for (const int dppChannel : generalChannels) {
    std::cout << "  DPP ch " << dppChannel
              << ": integrated charge = " << detector.LightIntegratedCharge(dppChannel) / unit::coulomb << " C\n";
  }
  for (const int dppChannel : pileupChannels) {
    std::cout << "  DPP ch " << dppChannel << " (pileup)"
              << ": integrated charge = " << detector.LightIntegratedCharge(dppChannel) / unit::coulomb << " C\n";
  }
  std::cout << "  photon count = " << detector.PhotonCount() << " (internal unit)"
            << ", event flags = 0x" << std::hex << detector.EventFlags() << std::dec << std::endl;
}

double NanoGRAMSAnalyzeLight::integratedROICharge(const LightData& lightData) const
{
  const grams::Config& cfg = *config_;
  const LightData::range_t preROI(-cfg.drift_time_max, -cfg.pre_roi_window);
  const LightData::range_t ROI(-cfg.pre_roi_window, cfg.post_roi_window);

  const double baseline = lightData.mean(preROI);
  if (!std::isfinite(baseline)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  // integral of (voltage - baseline) over the ROI
  const int numPoints = lightData.NumberOfPoints();
  const int start = std::clamp(lightData.findTimeIndex(ROI.first), 0, numPoints);
  const int stop = std::clamp(lightData.findTimeIndex(ROI.second), 0, numPoints);
  const int numSamples = std::max(0, stop - start);
  const double digitizerIntegral = lightData.integral(ROI) - baseline * numSamples * lightData.TimeWidth();

  // digitizer voltage -> output voltage of the transimpedance amplifier -> input current
  const double outputVoltageScale =
      (cfg.light_output_impedance_ohm + DigitizerInputImpedance) / DigitizerInputImpedance;
  return digitizerIntegral * outputVoltageScale / cfg.light_transimpedance_feedback_resistance_ohm;
}

std::vector<int> NanoGRAMSAnalyzeLight::collectValidChannels(const RealDetectorUnitNanoGRAMS& detector,
                                                             const std::vector<int>& channels) const
{
  std::vector<int> validChannels;
  for (const int dppChannel : channels) {
    if (dppChannel < detector.NumberOfLightData() && detector.getLightData(dppChannel)->isValid()) {
      validChannels.push_back(dppChannel);
    }
  }
  return validChannels;
}

NanoGRAMSAnalyzeLight::LightPeaks NanoGRAMSAnalyzeLight::analyzeChannelGroup(const RealDetectorUnitNanoGRAMS& detector,
                                                                             const std::vector<int>& channels) const
{
  const grams::Config& cfg = *config_;
  const LightData::range_t preROI(-cfg.drift_time_max, -cfg.pre_roi_window);
  const LightData::range_t ROI(-cfg.pre_roi_window, cfg.post_roi_window);
  const LightData::range_t postROI(cfg.post_roi_window, cfg.drift_time_max);

  // a missing peak (empty window) is treated as -infinity, i.e. never above the thresholds
  auto peakIn = [](const LightData& lightData, const LightData::range_t& range) {
    const double peak = lightData.peak(range);
    return std::isnan(peak) ? -std::numeric_limits<double>::infinity() : peak;
  };

  if (cfg.light_analysis_method == grams::LightAnalysisMethod::Average) {
    // the waveforms are averaged sample by sample; the time axis of the first channel is used
    const LightData& reference = *detector.getLightData(channels.front());
    for (const int dppChannel : channels) {
      if (detector.getLightData(dppChannel)->TimeWidth() != reference.TimeWidth()) {
        throw std::runtime_error(
            "light.waveform_analysis=average requires the same time width (wave_compress) in an analysis group.");
      }
    }
    LightData average(reference.NumberOfPoints(), reference.TimeWidth(), reference.TimeStart());
    std::vector<double>& averageWaveform = average.Waveform();
    for (const int dppChannel : channels) {
      const std::vector<double>& waveform = detector.getLightData(dppChannel)->Waveform();
      const std::size_t n = std::min(averageWaveform.size(), waveform.size());
      for (std::size_t i = 0; i < n; ++i) {
        averageWaveform[i] += waveform[i] / static_cast<double>(channels.size());
      }
    }
    return LightPeaks{peakIn(average, preROI), peakIn(average, ROI), peakIn(average, postROI)};
  }

  // each channel: the maximum over the channels
  LightPeaks peaks{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(),
                   -std::numeric_limits<double>::infinity()};
  for (const int dppChannel : channels) {
    const LightData& lightData = *detector.getLightData(dppChannel);
    peaks.preROI = std::max(peaks.preROI, peakIn(lightData, preROI));
    peaks.ROI = std::max(peaks.ROI, peakIn(lightData, ROI));
    peaks.postROI = std::max(peaks.postROI, peakIn(lightData, postROI));
  }
  return peaks;
}

} /* namespace comptonsoft */
