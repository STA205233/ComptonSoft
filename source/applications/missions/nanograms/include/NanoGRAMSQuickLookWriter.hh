/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************/

#ifndef COMPTONSOFT_NanoGRAMSQuickLookWriter_H
#define COMPTONSOFT_NanoGRAMSQuickLookWriter_H 1

#include <memory>
#include <string>
#include <vector>

#include "NanoGRAMSQuickLookTreeIO.hh"
#include "VCSModule.hh"

namespace comptonsoft
{

class NanoGRAMSReadTPCEvents;
class RealDetectorUnitNanoGRAMS;

/**
 * Writes the quicklook tree of the NanoGRAMS detector unit.
 * The event type is classified with the flags of the detector unit and the criteria of NanoGRAMSSelectEvents:
 *   Other (excluded core) > Gamma (a cluster survives the selection) > Cosmic > PileUp > TimeUp > Other.
 * Error events are not written since they are skipped by NanoGRAMSReadTPCEvents.
 *
 * This module should be placed after SelectHits and before NanoGRAMSSelectEvents
 * to write the events that are rejected by the selection.
 * @date 2026-09-24 | rewritten to take the data from RealDetectorUnitNanoGRAMS
 */
class NanoGRAMSQuickLookWriter : public VCSModule
{
  DEFINE_ANL_MODULE(NanoGRAMSQuickLookWriter, 2.0);

public:
  NanoGRAMSQuickLookWriter();
  ~NanoGRAMSQuickLookWriter() override;

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_end_run() override;

private:
  std::vector<int> selectedClusters() const;
  grams::TPCEventType classifyEvent(const std::vector<int>& selected_clusters) const;
  bool shouldWrite(grams::TPCEventType event_type, const std::vector<int>& selected_clusters) const;

  std::string quicklook_file_ = "quicklook.root";
  std::vector<std::string> event_types_;
  int num_hits_ = -1;
  bool save_waveforms_ = true;
  int output_flush_entries_ = 1000;

  const NanoGRAMSReadTPCEvents* tpc_events_ = nullptr;
  RealDetectorUnitNanoGRAMS* detector_ = nullptr;
  std::unique_ptr<grams::QuickLookTreeOutputWriter> writer_;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSQuickLookWriter_H */
