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

#include "SimDetectorUnitNanoGRAMS.hh"

#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <boost/format.hpp>

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandFlat.h"
#include "TRandom3.h"
#include "TH3.h"

#include "AstroUnits.hh"
#include "FlagDefinition.hh"
#include "DetectorHit.hh"

namespace unit = anlgeant4::unit;

namespace comptonsoft {

SimDetectorUnitNanoGRAMS::SimDetectorUnitNanoGRAMS()
{
  setTimingResolutionForTrigger(500.0 * unit::ns);
  setTimingResolutionForEnergyMeasurement(3000.0 * unit::ns);
}

SimDetectorUnitNanoGRAMS::~SimDetectorUnitNanoGRAMS() = default;

void SimDetectorUnitNanoGRAMS::initializeEvent()
{
  RealDetectorUnitLArTPCPixel::initializeEvent();
  DeviceSimulation::initializeEvent();
}

void SimDetectorUnitNanoGRAMS::prepareForTimingProcess() {
  simulatePulseHeights();
  auto& simulatedHits = getSimulatedHits();
  if (simulatedHits.empty()) {
    return;
  }
  for (auto& hit : simulatedHits) {
    const double drift_time = calculateDriftTime(hit->LocalPositionZ());
    hit->setTime(drift_time + hit->RealTime());
  }
  removeHitsOutOfPixelRange(simulatedHits);

  constexpr double SMALL_TIME = 1.0e-15 * unit::second;
  mergeHitsIfCoincident(SMALL_TIME, simulatedHits);
  sortHitsInTimeOrder(simulatedHits);
  mergeHitsIfCoincident(TimingResolutionForTrigger(), simulatedHits);
  performTriggerDiscrimination();
}

void SimDetectorUnitNanoGRAMS::makeDetectorHitsAtTime(double time_triggered, int time_group) {
  using HitType = DetectorHit_sptr;
  auto& SimulatedHits = getSimulatedHits();
  sortHitsInTimeOrder(SimulatedHits);
  for (auto& hit : SimulatedHits) {
   std::cout << boost::format("Simulated hit: time=%.2f ns, energy=%.2f keV, pixel=(%d, %d)")
     % (hit->Time() / unit::ns) % (hit->Energy() / unit::keV) % hit->Pixel().X() % hit->Pixel().Y()
     << std::endl;
  }
  const double time_start = time_triggered - 0.5*TimingResolutionForEnergyMeasurement();
  const double time_end   = time_triggered + 0.5*TimingResolutionForEnergyMeasurement();
  const auto itStart = std::find_if(std::begin(SimulatedHits),
                                    std::end(SimulatedHits),
                                    [=](HitType hit)-> bool {
                                      const double hitTime = hit->Time();
                                      return (hitTime >= time_start);
                                    });
  const auto itEnd = std::find_if(std::begin(SimulatedHits),
                                  std::end(SimulatedHits),
                                  [=](HitType hit)-> bool {
                                    const double hitTime = hit->Time();
                                    return (hitTime > time_end);
                                  });

  std::list<HitType> hits(itStart, itEnd);
  SimulatedHits.erase(std::begin(SimulatedHits), itEnd);
  mergeHits(hits);
  
  if (isPedestalEnabled()) {
    std::list<HitType> pedestalHits = generatePedestalSignals(time_group, time_end);
    std::move(pedestalHits.begin(), pedestalHits.end(), std::back_inserter(hits));
    mergeHits(hits);
  }
  
  removeHitsAtChannelsDisabled(hits);
  
  for (auto& hit: hits) {
    makeEPI(hit);
  }
  
  removeHitsBelowThresholds(hits);
  
  for (auto& hit: hits) {
    hit->setTimeGroup(time_group);
    hit->setTriggered(true);
    hit->setTriggeredTime(time_triggered);
    assignLocalDepth(hit);
    assignLocalPositionError(hit);
    insertDetectorHit(hit);
  }
}

void SimDetectorUnitNanoGRAMS::sortHitsInTimeOrder(std::list<DetectorHit_sptr>& hits)
{
  using HitType = DetectorHit_sptr;
  hits.sort([](HitType h1, HitType h2)-> bool {
      return h1->Time() < h2->Time();
    });
}

void SimDetectorUnitNanoGRAMS::performTriggerDiscrimination() {
  using HitType = DetectorHit_sptr;
  auto& SimulatedHits = getSimulatedHits();
  for (HitType& hit: SimulatedHits) {
    if (checkTriggerDiscrimination(hit->EnergyCharge(), hit->Pixel())) {
      hit->setSelfTriggered(true);
      hit->setSelfTriggeredTime(hit->Time());
    }
  }
}

void SimDetectorUnitNanoGRAMS::mergeHitsIfCoincident(double time_window, std::list<DetectorHit_sptr> &hits) {
  for (auto it1=hits.begin(); it1!=hits.end(); ++it1) {
    auto it2 = it1;
    ++it2;
    while ( it2 != hits.end() ) {
      const double hit1Time = (*it1)->Time();
      const double hit2Time = (*it2)->Time();
      if ( (*it1)->isInSamePixel(**it2) && std::abs(hit1Time-hit2Time)<=time_window ) {
        (*it1)->merge(**it2);
        it2 = hits.erase(it2);
      }
      else {
        ++it2;
      }
    }
  }
}

void SimDetectorUnitNanoGRAMS::printSimulationParameters(std::ostream& os) const
{
  os << "<SimDetectorUnitNanoGRAMS>" << '\n'
     << std::endl;
  DeviceSimulation::printSimulationParameters(os);
}

} /* namespace comptonsoft */
