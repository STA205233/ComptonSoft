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

#ifndef COMPTONSOFT_NanoGRAMSMakeLightWaveform_hh
#define COMPTONSOFT_NanoGRAMSMakeLightWaveform_hh 1

#include <anlnext/BasicModule.hh>

#include <cstdint>
#include <string>

namespace comptonsoft
{

class NanoGRAMSHitExtraction;
class NanoGRAMSLightWaveformStore;

class NanoGRAMSMakeLightWaveform : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(NanoGRAMSMakeLightWaveform, 1.0);

public:
  NanoGRAMSMakeLightWaveform();
  ~NanoGRAMSMakeLightWaveform() override;

  anlnext::ANLStatus mod_define()     override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze()    override;

private:
  bool hasFixedRange() const { return rangeMaxUs_ > rangeMinUs_; }

  std::string hitExtractionModuleName_ = "NanoGRAMSHitExtraction";
  std::string lightWaveformStoreModuleName_ = "NanoGRAMSLightWaveformStore";
  double rangeMinUs_ = 0.0;
  double rangeMaxUs_ = 0.0;

  NanoGRAMSHitExtraction* hitExtraction_ = nullptr;
  NanoGRAMSLightWaveformStore* lightWaveformStore_ = nullptr;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSMakeLightWaveform_hh */
