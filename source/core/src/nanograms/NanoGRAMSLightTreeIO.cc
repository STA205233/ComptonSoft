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

#include "NanoGRAMSLightTreeIO.hh"

#include <algorithm>
#include <iostream>
#include <limits>

#include "TTree.h"

namespace comptonsoft {

NanoGRAMSLightTreeIO::NanoGRAMSLightTreeIO() = default;
NanoGRAMSLightTreeIO::~NanoGRAMSLightTreeIO() = default;

void NanoGRAMSLightTreeIO::defineBranches()
{
  tree_->Branch("num_channels", &num_channels_, "num_channels/I");
  tree_->Branch("channels", channels_.data(), "channels[num_channels]/I");
  tree_->Branch("integrals", integrals_.data(), "integrals[num_channels]/D");
  tree_->Branch("total_integral", &total_integral_, "total_integral/D");
  tree_->Branch("peaks", peaks_.data(), "peaks[num_channels]/D");
  tree_->Branch("peak_pos", peak_pos_.data(), "peak_pos[num_channels]/D");
  tree_->Branch("total_peak", &total_peak_, "total_peak/D");
}

void NanoGRAMSLightTreeIO::setBranchAddresses()
{
  tree_->SetBranchAddress("num_channels", &num_channels_);
  tree_->SetBranchAddress("channels", channels_.data());
  tree_->SetBranchAddress("integrals", integrals_.data());
  tree_->SetBranchAddress("total_integral", &total_integral_);
  tree_->SetBranchAddress("peaks", peaks_.data());
  tree_->SetBranchAddress("peak_pos", peak_pos_.data());
  tree_->SetBranchAddress("total_peak", &total_peak_);
}

void NanoGRAMSLightTreeIO::fillEvent(const std::vector<int>& channels, const std::vector<double>& integrals, const std::vector<double>& peaks, const std::vector<double>& peak_pos, double total_integral, double total_peak)
{
  std::size_t n = std::min({channels.size(), integrals.size(), peaks.size(), peak_pos.size()});
  if (n > MaxChannels) {
    std::cerr << "NanoGRAMSLightTreeIO: too many channels (" << n << "); truncated to " << MaxChannels << std::endl;
    n = MaxChannels;
  }

  num_channels_ = static_cast<int32_t>(n);
  for (std::size_t i = 0; i < n; i++) {
    channels_[i] = static_cast<int32_t>(channels[i]);
    integrals_[i] = integrals[i];
    peak_pos_[i] = peak_pos[i];
    peaks_[i] = peaks[i];
  }
  total_integral_ = total_integral;
  total_peak_ = total_peak;

  tree_->Fill();
}

} /* namespace comptonsoft */
