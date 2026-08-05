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

#include "NanoGRAMSDppListDataProcessor.hh"

#include <iostream>

#include "TTree.h"

namespace comptonsoft
{
namespace grams
{

DppListTreeReader::DppListTreeReader(TTree* tree, int version, int verbose)
  : dppRootIO_(false, version),
    verbose_(verbose)
{
  dppRootIO_.SetVerbose(verbose);
  dppRootIO_.LoadTree(tree);
  dppRootIO_.SetBranchAddress();
  numEntries_ = dppRootIO_.GetEntries();
  group_.SetVerbose(verbose);

  hits_.fill(ngUtil::DppListDataDefinition(version));
  scratchHit_ = ngUtil::DppListDataDefinition(version);
  pendingHit_ = ngUtil::DppListDataDefinition(version);
}

DppListTreeReader::~DppListTreeReader() = default;

bool DppListTreeReader::processNext()
{
  group_.Clear();

  if (hasPendingHit_) {
    hasPendingHit_ = false;
    if (group_.AddEvent(pendingHit_)) {
      hits_[pendingHit_.GetChannel()] = pendingHit_;
    }
  }

  while (nextEntry_ < numEntries_) {
    dppRootIO_.GetEntry(nextEntry_);
    ++nextEntry_;
    dppRootIO_.FetchEvent(&scratchHit_);

    if (group_.AddEvent(scratchHit_)) {
      hits_[scratchHit_.GetChannel()] = scratchHit_;
      continue;
    }

    if (group_.IsFatal()) {
      // The hit is broken, or it conflicts with the group being built.
      // Discard the group and keep reading; a fatal flag on one event must not
      // stop the whole run.
      std::cerr << "DppListTreeReader: discarding the group at entry "
                << (nextEntry_ - 1) << " (channel " << scratchHit_.GetChannel()
                << ", trigger ID " << scratchHit_.GetTriggerID() << ")" << std::endl;
      group_.Clear();
      continue;
    }

    // The trigger ID changed: this hit belongs to the next event.
    pendingHit_ = scratchHit_;
    hasPendingHit_ = true;
    break;
  }

  if (group_.GetNumRegistered() == 0) {
    return false;
  }

  if (verbose_ > 0) {
    dumpCurrentGroup();
  }
  return true;
}

void DppListTreeReader::dumpCurrentGroup() const
{
  std::cout << "DppListTreeReader: trigger_id=" << group_.GetTriggerID()
            << " num_registered=" << group_.GetNumRegistered();
  for (int ch = 0; ch < NUM_CH_DPP_MAX; ch++) {
    if (!group_.IsRegistered(ch)) {
      continue;
    }
    std::cout << " [ch=" << ch
              << " wave_num=" << group_.GetWaveNum(ch)
              << " wave_compress=" << group_.GetWaveCompress(ch) << "]";
  }
  std::cout << std::endl;
}

} /* namespace grams */
} /* namespace comptonsoft */
