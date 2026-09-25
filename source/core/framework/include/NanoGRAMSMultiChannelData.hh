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

#ifndef COMPTONSOFT_NanoGRAMSMultiChannelData_hh
#define COMPTONSOFT_NanoGRAMSMultiChannelData_hh 1
#include <cstdint>
#include <memory>
#include <vector>
#include "MultiChannelData.hh"
namespace comptonsoft {

class NanoGRAMSTemperatureCorrection;

class NanoGRAMSMultiChannelData : public MultiChannelData
{
public:
  // number of VATA channels of one FEC
  static constexpr std::size_t NUM_CHANNELS = 64;
  static constexpr std::size_t NUM_FECS = 4;

public:
  NanoGRAMSMultiChannelData();
  virtual ~NanoGRAMSMultiChannelData();

  void resetEventData() override;

  /**
   * median of the valid channels. For an even number of channels,
   * the mean of the two central values is taken.
   * The common mode noise for selection (lower mean) is also calculated here.
   */
  double calculateCommonModeNoiseByMedian() override;

  /**
   * temperature (gain drift) correction: PHA is multiplied by the correction factor of this FEC
   * at the unixtime of the event.
   */
  void correctPHA() override;

  /**
   * convert PHA to EPI. The EPI for selection, which is based on the lower-mean
   * common mode noise, is also calculated.
   */
  bool convertPHA2EPI() override;

  /**
   * PHA (temperature corrected) -> charge (ADC2C gain function) -> EPI = number of electrons x W_ion
   */
  double PHA2EPI(std::size_t i, double pha) const override;

  /**
   * select hits by comparing the EPI for selection with the hit threshold.
   */
  void selectHits() override;

  // thresholds given by SelectHits are ignored; use setHitThresholdEnergy() instead.
  void resetThresholdEnergyVector(double) override {}
  void setThresholdEnergy(std::size_t, double) override {}
  void setThresholdEnergyVector(const std::vector<double>&) override {}
  void setHitThresholdEnergy(double v) { MultiChannelData::resetThresholdEnergyVector(v); }

  // common mode noise for hit selection: mean of the lowest N channels (before the median subtraction)
  void setNumLowerMeanChannels(int v) { numLowerMeanChannels_ = v; }
  int NumLowerMeanChannels() const { return numLowerMeanChannels_; }
  double getCommonModeNoiseForSelection() const { return commonModeNoiseForSelection_; }
  double getPHAForSelection(std::size_t i) const { return PHAForSelectionVector_[i]; }
  double getEPIForSelection(std::size_t i) const { return EPIForSelectionVector_[i]; }

  // temperature correction shared by the FECs of the detector; fec is the index of this FEC
  void setTemperatureCorrection(std::shared_ptr<const NanoGRAMSTemperatureCorrection> correction, int fec);
  // unixtime of the current event, given by the detector unit
  void setUnixTime(uint32_t v) { unixTime_ = v; }
  uint32_t UnixTime() const { return unixTime_; }
  // correction factor applied to PHA in the current event
  double temperatureCorrectionFactor() const { return temperatureCorrectionFactor_; }

  void setWion(double v) { Wion_ = v; }
  double Wion() const { return Wion_; }

private:
  std::shared_ptr<const NanoGRAMSTemperatureCorrection> temperatureCorrection_;
  int fec_;
  uint32_t unixTime_;
  double temperatureCorrectionFactor_;
  double Wion_;

  int numLowerMeanChannels_;
  double commonModeNoiseForSelection_;
  std::vector<double> PHAForSelectionVector_;
  std::vector<double> EPIForSelectionVector_;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_NanoGRAMSMultiChannelData_hh