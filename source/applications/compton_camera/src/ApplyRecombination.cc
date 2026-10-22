#include "ApplyRecombination.hh"
#include "CSException.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "DeviceSimulation.hh"
#include "boost/throw_exception.hpp"
#include <iostream>
#include <string>
#include <string_view>

using namespace anlnext;

namespace comptonsoft {
using SourceType = ApplyRecombination::SourceType;

ApplyRecombination::ApplyRecombination()
  : source_str_("edep"),
    source_(SourceType::EDEP),
    hit_collection_(nullptr)
{
}
ApplyRecombination::~ApplyRecombination() = default;

ANLStatus ApplyRecombination::mod_define()
{
  VCSModule::mod_define();
  define_parameter("source", &mod_class::source_str_);
  return AS_OK;
}
ANLStatus ApplyRecombination::mod_initialize()
{
  VCSModule::mod_initialize();
  try {
    source_ = parseSourceType(source_str_);
  }
  catch (const CSException& e) {
    std::cerr << e.getMessage() << std::endl;
    return AS_QUIT_ERROR;
  }

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

ANLStatus ApplyRecombination::mod_analyze()
{
  auto ds = getDetectorManager();
  const int n_time_group = hit_collection_->NumberOfTimeGroups();
  const auto getter = getGetter(source_);
  for (int i = 0; i < n_time_group; ++i) {
    auto& hits = hit_collection_->getHits(i);
    for (auto& hit : hits) {
      const double e = (hit.get()->*getter)();
      hit->setKineticEnergy(e);
      const auto* devicesim = ds->getDeviceSimulationByID(hit->DetectorID());
      devicesim->applyRecombination(hit);
    }
  }
  return AS_OK;
}

ApplyRecombination::SourceType ApplyRecombination::parseSourceType(std::string_view str)
{
  if (str == "epi") {
    return SourceType::EPI;
  }
  else if (str == "edep") {
    return SourceType::EDEP;
  }
  else if (str == "echarge") {
    return SourceType::ECHARGE;
  }
  else {
    BOOST_THROW_EXCEPTION(CSException("Invalid Source type: " + std::string(str)));
  }
}
ApplyRecombination::getter_func_ptr ApplyRecombination::getGetter(SourceType s)
{
  switch (s) {
  case SourceType::EPI:
    return &DetectorHit::EPI;
  case SourceType::ECHARGE:
    return &DetectorHit::EnergyCharge;
  case SourceType::EDEP:
    return &DetectorHit::EnergyDeposit;
  default:
    return &DetectorHit::EnergyDeposit;
  }
}

} // namespace comptonsoft