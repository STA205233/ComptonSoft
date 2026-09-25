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
 * @file NanoGRAMSTPCDataProcessor.hh
 * @brief TPC tree reader for NanoGRAMS data reduction.
 * @author Satoshi Takashima
 * @date 2026-05-17
 * @date 2026-09-24 | the event selection is moved to the detector unit; this reader only reads the tree
 */

#ifndef COMPTONSOFT_NanoGRAMSTPCDataProcessor_H
#define COMPTONSOFT_NanoGRAMSTPCDataProcessor_H 1

#include <cstdint>

#include "NanoGRAMSTPCTreeIO.hh"

class TTree;

namespace comptonsoft
{
namespace grams
{

/**
 * true if the TPC data of the event is usable,
 * i.e., the error flags (data inconsistency) are 0 or 4.
 */
bool isTPCDataUsable(int error_flags);

class TPCTreeReader
{
public:
  explicit TPCTreeReader(TTree* tpc_tree);
  ~TPCTreeReader();

  bool readNextEntry(int64_t& raw_event_id);
  const TPCTreeBuffer& currentBuffer() const { return tpc_tree_buffer_; }
  uint32_t currentUnixTime() const { return current_unix_time_; }

private:
  TPCTreeBuffer tpc_tree_buffer_;
  int64_t current_entry_ = 0;
  uint32_t current_unix_time_ = 0;
};

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSTPCDataProcessor_H */
