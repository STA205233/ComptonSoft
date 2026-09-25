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

#ifndef COMPTONSOFT_NanoGRAMSChannelMap_hh
#define COMPTONSOFT_NanoGRAMSChannelMap_hh 1

#include <array>
#include <memory>
#include <utility>

#include "NanoGRAMSMultiChannelData.hh"

namespace comptonsoft {

class ChannelMap;

namespace nanograms {

/**
 * Fixed channel map of the NanoGRAMS anode (16 x 16 pixels):
 * (section = FEC, channel = VATA channel) -> global pixel (x, y).
 * Each FEC reads an 8 x 8 section; the sections are placed at FECSectionOrigin.
 * @date 2026-09-24 | moved from NanoGRAMSConstants.hh/NanoGRAMSFECGeometry.cc (applications/missions/nanograms)
 */
constexpr int FECSectionSidePixels = 8;
constexpr int AnodeSidePixels = 2 * FECSectionSidePixels;

// Channel number at each FEC section grid point in image order:
// top-to-bottom rows, left-to-right columns.
constexpr std::array<std::array<int, NanoGRAMSMultiChannelData::NUM_CHANNELS>, NanoGRAMSMultiChannelData::NUM_FECS>
FECSectionGridToChannel = {{
    {0,  8, 23, 24, 39, 40, 55, 63,
     1,  9, 22, 25, 38, 41, 54, 62,
     2, 10, 21, 26, 37, 42, 53, 61,
     3, 11, 20, 27, 36, 43, 52, 60,
     4, 12, 19, 28, 35, 44, 51, 59,
     5, 13, 18, 29, 34, 45, 50, 58,
     6, 14, 17, 30, 33, 46, 49, 57,
     7, 15, 16, 31, 32, 47, 48, 56},
    {63, 62, 61, 60, 59, 58, 57, 56,
     55, 54, 53, 52, 51, 50, 49, 48,
     40, 41, 42, 43, 44, 45, 46, 47,
     39, 38, 37, 36, 35, 34, 33, 32,
     24, 25, 26, 27, 28, 29, 30, 31,
     23, 22, 21, 20, 19, 18, 17, 16,
     8,   9, 10, 11, 12, 13, 14, 15,
     0,   1,  2,  3,  4,  5,  6,  7},
    {56, 48, 47, 32, 31, 16, 15, 7,
     57, 49, 46, 33, 30, 17, 14, 6,
     58, 50, 45, 34, 29, 18, 13, 5,
     59, 51, 44, 35, 28, 19, 12, 4,
     60, 52, 43, 36, 27, 20, 11, 3,
     61, 53, 42, 37, 26, 21, 10, 2,
     62, 54, 41, 38, 25, 22,  9, 1,
     63, 55, 40, 39, 24, 23,  8, 0},
    { 7,  6,  5,  4,  3,  2,  1, 0,
     15, 14, 13, 12, 11, 10,  9,  8,
     16, 17, 18, 19, 20, 21, 22, 23,
     31, 30, 29, 28, 27, 26, 25, 24,
     32, 33, 34, 35, 36, 37, 38, 39,
     47, 46, 45, 44, 43, 42, 41, 40,
     48, 49, 50, 51, 52, 53, 54, 55,
     56, 57, 58, 59, 60, 61, 62, 63}
}};

// global pixel (x, y) of the lower-left corner of each FEC section
constexpr std::array<std::pair<int, int>, NanoGRAMSMultiChannelData::NUM_FECS> FECSectionOrigin = {{
    {0, 0},
    {FECSectionSidePixels, 0},
    {FECSectionSidePixels, FECSectionSidePixels},
    {0, FECSectionSidePixels},
}};

// section coordinate (x, y) of a grid point; y = 0 at the bottom row
constexpr std::pair<int, int> sectionCoordinate(int row_from_top, int col_from_left)
{
  return {col_from_left, FECSectionSidePixels - 1 - row_from_top};
}

std::shared_ptr<ChannelMap> makeChannelMap();

} /* namespace nanograms */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSChannelMap_hh */
