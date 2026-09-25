#include "NanoGRAMSMultiChannelData.hh"
#include "AstroUnits.hh"
#include "CSException.hh"
#include "NanoGRAMSTemperatureCorrection.hh"
#include "VGainFunction.hh"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>
#include <memory>
namespace unit = anlgeant4::unit;

namespace comptonsoft {

NanoGRAMSMultiChannelData::NanoGRAMSMultiChannelData()
  : MultiChannelData(NUM_CHANNELS, ElectrodeSide::Anode),
    fec_(0),
    unixTime_(0u),
    temperatureCorrectionFactor_(1.0),
    Wion_(23.6 * unit::eV),
    numLowerMeanChannels_(10),
    commonModeNoiseForSelection_(0.0),
    PHAForSelectionVector_(NUM_CHANNELS, 0.0),
    EPIForSelectionVector_(NUM_CHANNELS, 0.0)
{
  setUseNegativePulse(false);
  setPrioritySide(true);
}

NanoGRAMSMultiChannelData::~NanoGRAMSMultiChannelData() = default;

void NanoGRAMSMultiChannelData::setTemperatureCorrection(std::shared_ptr<const NanoGRAMSTemperatureCorrection> correction,
                                                         int fec)
{
  temperatureCorrection_ = correction;
  fec_ = fec;
}

void NanoGRAMSMultiChannelData::resetEventData()
{
  MultiChannelData::resetEventData();
  unixTime_ = 0u;
  temperatureCorrectionFactor_ = 1.0;
  commonModeNoiseForSelection_ = 0.0;
  std::fill(PHAForSelectionVector_.begin(), PHAForSelectionVector_.end(), 0.0);
  std::fill(EPIForSelectionVector_.begin(), EPIForSelectionVector_.end(), 0.0);
}

double NanoGRAMSMultiChannelData::calculateCommonModeNoiseByMedian()
{
  std::vector<double> sortedPHA;
  for (std::size_t i = 0; i < NumberOfChannels(); i++) {
    if (getDataValid(i) && getChannelEnabled(i)) {
      sortedPHA.push_back(getPHA(i));
    }
  }
  const std::size_t n = sortedPHA.size();
  if (n < 1) {
    return 0.0;
  }

  std::sort(sortedPHA.begin(), sortedPHA.end());
  const double median = (n % 2 == 0) ? 0.5 * (sortedPHA[n / 2 - 1] + sortedPHA[n / 2]) : sortedPHA[n / 2];
  setCommonModeNoise(median);

  // common mode noise for selection: mean of the lowest channels
  const std::size_t numLower = std::clamp<std::size_t>(numLowerMeanChannels_, 1, n);
  commonModeNoiseForSelection_ =
      std::accumulate(sortedPHA.begin(), sortedPHA.begin() + numLower, 0.0) / static_cast<double>(numLower);

  return median;
}

void NanoGRAMSMultiChannelData::correctPHA()
{
  temperatureCorrectionFactor_ =
      temperatureCorrection_ ? temperatureCorrection_->factor(fec_, static_cast<double>(unixTime_)) : 1.0;
  for (std::size_t i = 0; i < NumberOfChannels(); i++) {
    if (getDataValid(i) && getChannelEnabled(i)) {
      setPHA(i, getPHA(i) * temperatureCorrectionFactor_);
    }
  }
}

bool NanoGRAMSMultiChannelData::convertPHA2EPI()
{
  // PHA for selection = (raw ADC - lower mean) x temperature correction factor
  //                   = PHA + (CMN - lower mean) x temperature correction factor
  const double offset = (getCommonModeNoise() - commonModeNoiseForSelection_) * temperatureCorrectionFactor_;
  for (std::size_t i = 0; i < NumberOfChannels(); i++) {
    if (getDataValid(i) && getChannelEnabled(i)) {
      PHAForSelectionVector_[i] = getPHA(i) + offset;
      EPIForSelectionVector_[i] = PHA2EPI(i, PHAForSelectionVector_[i]);
    }
    else {
      PHAForSelectionVector_[i] = 0.0;
      EPIForSelectionVector_[i] = 0.0;
    }
  }

  return MultiChannelData::convertPHA2EPI();
}

void NanoGRAMSMultiChannelData::selectHits()
{
  for (std::size_t i = 0; i < NumberOfChannels(); i++) {
    if (getDataValid(i) && getChannelEnabled(i) && discriminate(i, getEPIForSelection(i))) {
      setChannelHit(i, 1);
    }
    else {
      setChannelHit(i, 0);
    }
  }
}

double NanoGRAMSMultiChannelData::PHA2EPI(std::size_t i, double pha) const
{
  if (i >= NumberOfChannels()) {
    throw CSException("Channel index is out of range");
  }
  // NaN (e.g., temperature correction not available) is propagated
  if (std::isnan(pha)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  if (!std::isfinite(pha) || pha <= 0.0) {
    return 0.0;
  }

  const std::shared_ptr<const VGainFunction> gainFunction = getGainFunction(i);
  if (!gainFunction) {
    throw CSException("Gain function is not set");
  }
  const double charge = gainFunction->eval(pha) - gainFunction->eval(0.0);
  if (charge <= 0.0) {
    return 0.0;
  }

  const double num_electrons = charge * unit::coulomb / unit::eplus;
  return num_electrons * Wion_;
}

} // namespace comptonsoft