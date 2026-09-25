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

#include "NanoGRAMSChannelMap.hh"
#include "ChannelMap.hh"

namespace comptonsoft {
namespace nanograms {

std::shared_ptr<ChannelMap> makeChannelMap()
{
  auto channelMap = std::make_shared<ChannelMap>(NanoGRAMSMultiChannelData::NUM_FECS,
                                                 NanoGRAMSMultiChannelData::NUM_CHANNELS,
                                                 AnodeSidePixels, AnodeSidePixels);
  for (std::size_t fec = 0; fec < NanoGRAMSMultiChannelData::NUM_FECS; ++fec) {
    const auto [originX, originY] = FECSectionOrigin[fec];
    for (int row = 0; row < FECSectionSidePixels; ++row) {
      for (int col = 0; col < FECSectionSidePixels; ++col) {
        const int channel = FECSectionGridToChannel[fec][row * FECSectionSidePixels + col];
        const auto [x, y] = sectionCoordinate(row, col);
        channelMap->set(fec, channel, originX + x, originY + y);
      }
    }
  }
  return channelMap;
}

} /* namespace nanograms */
} /* namespace comptonsoft */
