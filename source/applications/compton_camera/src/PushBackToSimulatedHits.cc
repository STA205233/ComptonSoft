#include "PushBackToSimulatedHits.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "DeviceSimulation.hh"
using namespace anlnext;
namespace comptonsoft {
PushBackToSimulatedHits::PushBackToSimulatedHits()
  : hit_collection_(nullptr)
{
}
PushBackToSimulatedHits::~PushBackToSimulatedHits() = default;

ANLStatus PushBackToSimulatedHits::mod_initialize()
{
  VCSModule::mod_initialize();
  constexpr char collec_name[] = "CSHitCollection";
  if (exist_module(collec_name)) {
    get_module_NC(collec_name, &hit_collection_);
  }
  else {
    return AS_QUIT_ERROR;
  }
  return AS_OK;
}
ANLStatus PushBackToSimulatedHits::mod_analyze()
{
  auto ds = getDetectorManager();
  const int n_time_group = hit_collection_->NumberOfTimeGroups();
  for (int i = 0; i < n_time_group; ++i) {
    auto& hits = hit_collection_->getHits(i);
    for (auto& hit : hits) {
      auto* devicesim = ds->getDeviceSimulationByID(hit->DetectorID());
      devicesim->insertSimulatedHit(hit);
    }
    hits.clear();
  }
  return AS_OK;
}

} // namespace comptonsoft