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

#ifndef COMPTONSOFT_NanoGRAMSReadDppList_H
#define COMPTONSOFT_NanoGRAMSReadDppList_H 1

#include "VCSModule.hh"

#include <memory>
#include <string>

#include "NanoGRAMSConfig.hh"

class TChain;

namespace comptonsoft {

class NanoGRAMSLightWaveformStore;

namespace grams {
class DppListTreeReader;
}

/**
 * Read light waveforms from a dpplist file and push them into
 * NanoGRAMSLightWaveformStore. This drives the ANL event loop, and is the
 * dpplist counterpart of NanoGRAMSHitExtraction + NanoGRAMSMakeLightWaveform.
 *
 * One trigger ID makes one event. The dpplist data has no TPC information, so
 * no hit is produced.
 *
 * @author Shota Arai
 * @date 2026-07-30
 */
class NanoGRAMSReadDppList : public VCSModule
{
  DEFINE_ANL_MODULE(NanoGRAMSReadDppList, 1.0);

public:
  NanoGRAMSReadDppList();
  ~NanoGRAMSReadDppList();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_finalize() override;

private:
  bool hasFixedRange() const { return rangeMinUs_ < rangeMaxUs_; }

  std::vector<std::string> dppListFile_;
  std::string configFile_;
  std::unique_ptr<TChain> chain_;
  /// DPP readout configuration providing savefile.listwave_delay.
  std::string lightWaveformStoreModuleName_;
  int version_ = 2;
  int verbose_ = 0;
  double rangeMinUs_ = 0.0;
  double rangeMaxUs_ = 0.0;

  grams::Config cfg_;
  std::unique_ptr<grams::DppListTreeReader> reader_;
  NanoGRAMSLightWaveformStore* lightWaveformStore_ = nullptr;
  int64_t processedEvents_ = 0;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSReadDppList_H */
