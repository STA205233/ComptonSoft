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

#include "NanoGRAMSLightWaveformQuery.hh"

#include "NanoGRAMSEvent.hh"
#include "NanoGRAMSLightWaveformStore.hh"

#include <algorithm>
#include <iostream>

using namespace anlnext;

namespace comptonsoft
{

VNanoGRAMSLightWaveformQuery::VNanoGRAMSLightWaveformQuery() = default;

VNanoGRAMSLightWaveformQuery::~VNanoGRAMSLightWaveformQuery() = default;

ANLStatus VNanoGRAMSLightWaveformQuery::mod_define()
{
  define_parameter("config_file",                      &mod_class::configFile_);
  define_parameter("light_waveform_store_module_name", &mod_class::lightWaveformStoreModuleName_);
  define_parameter("include_evs", &mod_class::includeEvs_);
  define_parameter("exclude_evs", &mod_class::excludeEvs_);
  return AS_OK;
}

ANLStatus VNanoGRAMSLightWaveformQuery::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  if (exist_module(lightWaveformStoreModuleName_)) {
    get_module_NC(lightWaveformStoreModuleName_, &lightWaveformStore_);
  }
  else {
    std::cerr << "VNanoGRAMSLightWaveformQuery::mod_initialize: "
              << lightWaveformStoreModuleName_ << " module not found" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  grams::readConfig(cfg_, configFile_);

  channels_ = cfg_.general_analysis_channels;
  channels_.insert(channels_.end(),
                   cfg_.pileup_analysis_channels.begin(),
                   cfg_.pileup_analysis_channels.end());
  channels_.erase(std::remove_if(channels_.begin(), channels_.end(),
                                 [](int ch) {
                                   return ch < 0 || ch >= NUM_CH_DPP_MAX;
                                 }),
                  channels_.end());
  std::sort(channels_.begin(), channels_.end());
  channels_.erase(std::unique(channels_.begin(), channels_.end()), channels_.end());

  return AS_OK;
}

ANLStatus VNanoGRAMSLightWaveformQuery::mod_begin_run()
{
  // Check of EVS
  for (const auto& include_evs: includeEvs_) {
    if (!is_evs_defined(include_evs)) {
      std::cerr << module_id() << ": EVS " << include_evs << " is not defined" << std::endl;
      return AS_QUIT_ERROR;
    }
  }
  for (const auto& exclude_evs: excludeEvs_) {
    if (!is_evs_defined(exclude_evs)) {
      std::cerr << module_id() << ": EVS " << exclude_evs << " is not defined" << std::endl;
      return AS_QUIT_ERROR;
    }
  }
  return AS_OK;
}

bool VNanoGRAMSLightWaveformQuery::isSkipLoop() const
{
  bool has_include = false;
  bool has_exclude = false;
  for (const auto& include_evs : includeEvs_) {
    if (evs(include_evs)) {
      has_include = true;
      break;
    }
  }
  if (!has_include && !includeEvs_.empty()) {
    return true;
  }
  for (const auto& exclude_evs : excludeEvs_) {
    if (evs(exclude_evs)) {
      has_exclude = true;
      break;
    }
  }
  if (has_exclude) {
    return true;
  }
  return false;
}

} /* namespace comptonsoft */
