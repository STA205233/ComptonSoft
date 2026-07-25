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

#ifndef COMPTONSOFT_NanoGRAMSCorrectDigitizerOffset_hh
#define COMPTONSOFT_NanoGRAMSCorrectDigitizerOffset_hh 1

#include "NanoGRAMSLightWaveformStage.hh"

namespace comptonsoft
{

class NanoGRAMSCorrectDigitizerOffset : public VNanoGRAMSLightWaveformStage
{
  DEFINE_ANL_MODULE(NanoGRAMSCorrectDigitizerOffset, 1.0);

public:
  NanoGRAMSCorrectDigitizerOffset();
  ~NanoGRAMSCorrectDigitizerOffset() override;

  anlnext::ANLStatus mod_define() override;

protected:
  void applyCorrection(std::vector<double>& waveform, int wave_compress) override;

private:
  int rangeStartIndex_ = 0;
  int rangeStopIndex_ = 0;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSCorrectDigitizerOffset_hh */
