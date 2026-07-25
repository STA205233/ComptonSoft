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

#include "NanoGRAMSLightWaveform.hh"

#include "DppListDataDefinition.hh"
#include "NanoGRAMSTPCTreeIO.hh"


namespace comptonsoft
{
namespace grams
{

LightWaveformView viewFromTPCTree(const TPCTreeBuffer& buf, int light_ch)
{
  const int waveform_slot = buf.waveformSlotForDPPChannel(light_ch);
  if (waveform_slot < 0) {
    return LightWaveformView{};
  }

  const int waveform_len = buf.layout().waveform_len;
  const int waveform_offset = waveform_slot * waveform_len;

  LightWaveformView view;
  view.samples = buf.waveform.data() + waveform_offset;
  view.length = waveform_len;
  view.wave_compress = static_cast<int>(buf.wave_compress[light_ch]);
  return view;
}

LightWaveformView viewFromDppListHit(const ngUtil::DppListDataDefinition& hit)
{
  LightWaveformView view;
  view.samples = hit.GetWaveData().data();
  view.length = static_cast<int>(hit.GetWaveNum());
  view.wave_compress = static_cast<int>(hit.GetWaveCompress());
  return view;
}

} /* namespace grams */
} /* namespace comptonsoft */
