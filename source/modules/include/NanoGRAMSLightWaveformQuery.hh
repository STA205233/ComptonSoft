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

#ifndef COMPTONSOFT_NanoGRAMSLightWaveformQuery_hh
#define COMPTONSOFT_NanoGRAMSLightWaveformQuery_hh 1

#include <string>
#include <vector>

#include "NanoGRAMSConfig.hh"
#include "VCSModule.hh"

namespace comptonsoft
{

class NanoGRAMSLightWaveformStore;

class VNanoGRAMSLightWaveformQuery : public VCSModule
{
DEFINE_ANL_MODULE(VNanoGRAMSLightWaveformQuery, "1.0")
public:
  VNanoGRAMSLightWaveformQuery();
  ~VNanoGRAMSLightWaveformQuery() override;

  anlnext::ANLStatus mod_define()     override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_begin_run() override;

protected:
  std::string configFile_ = "";
  std::string lightWaveformStoreModuleName_ = "NanoGRAMSLightWaveformStore";
  grams::Config cfg_;
  NanoGRAMSLightWaveformStore* lightWaveformStore_ = nullptr;
  std::vector<int> channels_;
  bool isSkipLoop() const;

private:
  std::vector<std::string> includeEvs_;
  std::vector<std::string> excludeEvs_;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSLightWaveformQuery_hh */
