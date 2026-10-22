#include "ApplyCCE.hh"
#include "CLHEP/Vector/ThreeVector.h"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "DeviceSimulation.hh"
#include "VRealDetectorUnit.hh"
#include <iostream>

using namespace anlnext;

namespace comptonsoft {
ApplyCCE::ApplyCCE()
  : hit_collection_(nullptr)
{
}
ApplyCCE::~ApplyCCE() = default;

ANLStatus ApplyCCE::mod_initialize()
{
  VCSModule::mod_initialize();
  constexpr char collection_name[] = "CSHitCollection";
  if (exist_module(collection_name)) {
    get_module_NC(collection_name, &hit_collection_);
  }
  else {
    std::cerr << collection_name << " not found" << std::endl;
    return AS_QUIT_ERROR;
  }
  return AS_OK;
}

ANLStatus ApplyCCE::mod_analyze()
{
  auto ds = getDetectorManager();
  const int n_time_group = hit_collection_->NumberOfTimeGroups();
  for (int i = 0; i < n_time_group; ++i) {
    auto& hits = hit_collection_->getHits(i);
    for (auto& hit : hits) {
      const double echarge = hit->EnergyCharge();
      const auto detector_id = hit->DetectorID();
      const auto pixel = hit->Pixel();
      const auto real_pos = hit->RealPosition();

      const auto* devicesim = ds->getDeviceSimulationByID(detector_id);
      const auto detector = ds->getDetectorByID(detector_id);
      const auto local_pos =
          detector->globalToLocal(real_pos); // local_pos is already fluctuated; convert from real_pos

      const double echarge_converted =
          devicesim->calculateEnergyCharge(pixel, echarge, local_pos.x(), local_pos.y(), local_pos.z());
      hit->setEnergyCharge(echarge_converted);
    }
  }
  return AS_OK;
}

} // namespace comptonsoft