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

#ifndef COMPTONSOFT_SimDetectorUnitVATA_H
#define COMPTONSOFT_SimDetectorUnitVATA_H 1

#include "SimDetectorUnitLArTPCPixel.hh"

class TH3D;

namespace comptonsoft {

/**
 * A class of a VATA detector unit including device simulations.
 * @author Shota Arai
 * @date 2026-06-08
 */
class SimDetectorUnitNanoGRAMS
  : public SimDetectorUnitLArTPCPixel
{
public:
  SimDetectorUnitNanoGRAMS();
  virtual ~SimDetectorUnitNanoGRAMS();

  void initializeEvent() override;

  void printSimulationParameters(std::ostream& os) const override;
  void prepareForTimingProcess() override;
  void makeDetectorHitsAtTime(double time_triggered, int time_group) override;
  
protected:
  
private:
  void sortHitsInTimeOrder(std::list<DetectorHit_sptr>& hits);
  void performTriggerDiscrimination();
  void mergeHitsIfCoincident(double time_window, std::list<DetectorHit_sptr>& hits);
  
};
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_SimDetectorUnitVATA_H */
