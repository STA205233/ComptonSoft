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

/**
 * @file NanoGRAMSDppListDataProcessor.hh
 * @brief Event-wise reader of the dpplist tree.
 * @author Shota Arai
 * @date 2026-07-30
 */

#ifndef COMPTONSOFT_NanoGRAMSDppListDataProcessor_H
#define COMPTONSOFT_NanoGRAMSDppListDataProcessor_H 1

#include <array>
#include <cstdint>

#include "DppListDataDefinition.hh"
#include "DppRootIO.hh"
#include "GroupedDppListData.hh"
#include "NanoGRAMSEvent.hh"

class TTree;

namespace comptonsoft
{
namespace grams
{

/**
 * Reads the dpplist tree, where one entry corresponds to one channel hit, and
 * groups the entries into events by the trigger ID.
 * This is the dpplist counterpart of TPCTreeReader.
 */
class DppListTreeReader
{
public:
  DppListTreeReader(TTree* tree, int version = 2, int verbose = 0);
  ~DppListTreeReader();

  DppListTreeReader(const DppListTreeReader&) = delete;
  DppListTreeReader& operator=(const DppListTreeReader&) = delete;

  int64_t nEntries() const { return numEntries_; }

  /**
   * Read the next group of entries sharing the same trigger ID.
   * @return false when the tree is exhausted.
   */
  bool processNext();

  uint32_t currentTriggerId() const { return group_.GetTriggerID(); }
  int numberOfRegisteredChannels() const { return group_.GetNumRegistered(); }
  bool isRegistered(int ch) const { return group_.IsRegistered(ch); }

  /**
   * @return the raw hit of the channel, to be passed to viewFromDppListHit().
   * Valid only when isRegistered(ch) is true.
   */
  const ngUtil::DppListDataDefinition& hit(int ch) const { return hits_[ch]; }

  const ngUtil::GroupedDppListData& group() const { return group_; }

private:
  void dumpCurrentGroup() const;

  ngUtil::DppRootIO dppRootIO_;
  int verbose_ = 0;
  int64_t numEntries_ = 0;
  int64_t nextEntry_ = 0;

  /// Reused across events so that no allocation happens in the event loop.
  ngUtil::GroupedDppListData group_;
  std::array<ngUtil::DppListDataDefinition, NUM_CH_DPP_MAX> hits_;

  /// Destination of FetchEvent(), reused so that the waveform buffer is not
  /// reallocated for every entry.
  ngUtil::DppListDataDefinition scratchHit_;

  /// A hit that belongs to the next event, read ahead while closing a group.
  ngUtil::DppListDataDefinition pendingHit_;
  bool hasPendingHit_ = false;
};

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSDppListDataProcessor_H */
