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

#ifndef COMPTONSOFT_RealDetectorUnitNanoGRAMS_hh
#define COMPTONSOFT_RealDetectorUnitNanoGRAMS_hh 1
#include "RealDetectorUnitLArTPCPixel.hh"
namespace comptonsoft {
class VRealDetectorUnit;
class RealDetectorUnitNanoGRAMS : public RealDetectorUnitLArTPCPixel
{
public:
  RealDetectorUnitNanoGRAMS();
  virtual ~RealDetectorUnitNanoGRAMS();

  DetectorType Type() const override { return DetectorType::NanoGRAMS; }
  ElectrodeSide ReadoutElectrode() const { return ElectrodeSide::Anode; }
  bool isAnodeReadout() const { return true; }
  bool isCathodeReadout() const { return false; }

  bool isBottomSideReadout() const { return false; }
  bool isUpSideReadout() const { return true; }
  void reconstructHits() override;
};
} // namespace comptonsoft
#endif