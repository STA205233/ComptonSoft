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


#ifndef COMPTONSOFT_NanoGRAMSLightWaveform_H
#define COMPTONSOFT_NanoGRAMSLightWaveform_H 1

#include "DppListDataDefinition.hh"
#include <vector>
#include <cstdint>

namespace ngutil {
class DppListDataDefinition;
}

namespace comptonsoft
{
namespace grams
{

struct LightWaveform
{
  std::vector<double> values;
  double xlow_us = 0.0;
  double xhigh_us = 0.0;
};

class TPCTreeBuffer;

struct LightWaveformView
{
  const int16_t* samples = nullptr;
  int length = 0;
  int wave_compress = 1;
};

LightWaveformView viewFromTPCTree(const TPCTreeBuffer& buf, int light_ch);
LightWaveformView viewFromDppListHit(const ngUtil::DppListDataDefinition& hit);


} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSStoredLightWaveform_H */
