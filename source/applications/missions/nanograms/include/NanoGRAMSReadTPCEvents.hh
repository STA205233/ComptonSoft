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

#ifndef COMPTONSOFT_NanoGRAMSReadTPCEvents_H
#define COMPTONSOFT_NanoGRAMSReadTPCEvents_H 1

#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NanoGRAMSCalibrationData.hh"
#include "NanoGRAMSLoadConfig.hh"
#include "NanoGRAMSTPCDataProcessor.hh"
#include "VCSModule.hh"

class TFile;

namespace comptonsoft {

class NanoGRAMSTemperatureCorrection;

class NanoGRAMSReadTPCEvents : public VCSModule
{
  DEFINE_ANL_MODULE(NanoGRAMSReadTPCEvents, 1.0);

public:
  NanoGRAMSReadTPCEvents();
  ~NanoGRAMSReadTPCEvents() override;

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_end_run() override;

  int64_t currentRawEventId() const { return current_raw_event_id_; }
  int32_t runId() const { return run_id_; }
  uint32_t currentUnixTime() const { return current_unix_time_; }
  const grams::TPCTreeBuffer& currentTPCBuffer() const { return tpc_tree_reader_->currentBuffer(); }

private:
  bool readNextTPCEvent(int64_t& raw_event_id);
  bool openNextTPCFile();
  void fillEventsIntoDetectors();
  void setupLightDataLayout();
  void selectLightChannels();
  std::vector<std::string> tpctree_files_;
  int32_t run_id_ = 0;

  std::unique_ptr<TFile> input_file_;
  std::unique_ptr<grams::TPCTreeReader> tpc_tree_reader_;
  std::size_t input_file_index_ = 0;
  int64_t skipped_error_events_ = 0;
  int64_t processed_entries_ = 0;
  int64_t expected_tpc_entries_ = 0;
  int64_t current_raw_event_offset_ = 0;
  int64_t current_raw_event_id_ = -1;
  uint32_t current_unix_time_ = 0;
  // DPP channels used in the light analysis (general or pileup); only these are filled into light data
  std::array<bool, NUM_CH_DPP_MAX> light_channel_used_{};

  NanoGRAMSLoadConfig* configLoader_ = nullptr;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSReadTPCEvents_H */
