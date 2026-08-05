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

#ifndef COMPTONSOFT_NanoGRAMSLightTreeIO_H
#define COMPTONSOFT_NanoGRAMSLightTreeIO_H 1

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "NanoGRAMSEvent.hh"

class TTree;

namespace comptonsoft {

/**
 * Tree IO of the light waveform integrals for each analysis channel.
 * One entry corresponds to one event so that the tree can be used as
 * a friend tree of the event tree.
 *
 * @author Shota Arai
 * @date 2026-07-29
 */
class NanoGRAMSLightTreeIO
{
public:
  static const std::size_t MaxChannels = NUM_CH_DPP_MAX;

public:
  NanoGRAMSLightTreeIO();
  virtual ~NanoGRAMSLightTreeIO();

  virtual void setTree(TTree* tree) { tree_ = tree; }

  virtual void defineBranches();
  virtual void setBranchAddresses();

  void fillEvent(const std::vector<int>& channels, const std::vector<double>& integrals, const std::vector<double>& peaks, const std::vector<double>& peak_pos, double total_integrals, double total_peak);

  int32_t getNumberOfChannels() const
  {
    return num_channels_;
  }
  int32_t getChannel(std::size_t i) const { return channels_[i]; }
  double getIntegral(std::size_t i) const { return integrals_[i]; }
  double getTotalIntegral() const { return total_integral_; }
  double getPeak(std::size_t i) const { return peaks_[i]; }
  double getPeakPosition(std::size_t i) const { return peak_pos_[i]; }
  double getTotalPeak() const { return total_peak_; }

private:
  TTree* tree_ = nullptr;

  /*
   * tree contents
   */
  int32_t num_channels_ = 0;
  std::array<int32_t, MaxChannels> channels_;
  std::array<double, MaxChannels> integrals_;
  double total_integral_ = 0.0;
  std::array<double, MaxChannels> peaks_;
  std::array<double, MaxChannels> peak_pos_;
  double total_peak_ = 0.0;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightTreeIO_H */
