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

#include "RecalculateSimulationNoise.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "DeviceSimulation.hh"
#include "TRandom3.h"

using namespace anlnext;

namespace comptonsoft {

RecalculateSimulationNoise::RecalculateSimulationNoise()
  : m_HitCollection(nullptr),
    m_src_str("DetectorHit"),
    m_src(SOURCE::DetectorHit)
{
}

ANLStatus RecalculateSimulationNoise::mod_define()
{
  VCSModule::mod_define();
  define_parameter("src", &mod_class::m_src_str);
  return AS_OK;
}

ANLStatus RecalculateSimulationNoise::mod_initialize()
{
  VCSModule::mod_initialize();

  get_module_NC("CSHitCollection", &m_HitCollection);
  if (m_src_str == "DetectorHit") {
    m_src = SOURCE::DetectorHit;
  }
  else if (m_src_str == "SimulatedHit") {
    m_src = SOURCE::SimulatedHit;
  }
  else {
    throw CSException("Unknown Source: " + m_src_str);
  }

  return AS_OK;
}

ANLStatus RecalculateSimulationNoise::mod_analyze()
{
  DetectorSystem* detectorManager = getDetectorManager();
  if (m_src == SOURCE::DetectorHit) {
    const int NumTimeGroups = m_HitCollection->NumberOfTimeGroups();
    for (int timeGroup = 0; timeGroup < NumTimeGroups; timeGroup++) {
      std::vector<DetectorHit_sptr>& hits = m_HitCollection->getHits(timeGroup);
      for (DetectorHit_sptr& hit : hits) {
        const int detectorID = hit->DetectorID();
        DeviceSimulation* ds = detectorManager->getDeviceSimulationByID(detectorID);

        ds->makeEPI(hit);
      }
    }
  }
  else if (m_src == SOURCE::SimulatedHit) {
    for (auto& detector : detectorManager->getDetectors()) {
      DeviceSimulation* ds = detectorManager->getDeviceSimulationByID(detector->getID());
      for (int i = 0; i < detector->NumberOfDetectorHits(); ++i) {
        ds->makeEPI(detector->getDetectorHit(i));
      }
    }
  }
  return AS_OK;
}

} /* namespace comptonsoft */
