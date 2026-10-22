#include "EliminateLowEnergyHits.hh"
#include "DetectorHit.hh"
using namespace anlnext;

namespace comptonsoft {
EliminateLowEnergyHits::EliminateLowEnergyHits()
  : BasicModule(),
    m_HitCollection(nullptr)
{
}
EliminateLowEnergyHits::~EliminateLowEnergyHits() = default;
EliminateLowEnergyHits::EliminateLowEnergyHits(const EliminateLowEnergyHits& r) = default;

ANLStatus EliminateLowEnergyHits::mod_define()
{
  define_parameter("photon_threshold", &mod_class::m_PhotonThreshold);
  define_parameter("energy_threshold", &mod_class::m_EnergyThreshold, CLHEP::keV, "keV");
  return AS_OK;
}
ANLStatus EliminateLowEnergyHits::mod_initialize()
{
  get_module_NC("CSHitCollection", &m_HitCollection);
  return AS_OK;
}
ANLStatus EliminateLowEnergyHits::mod_analyze()
{
  const int NumTimeGroups = m_HitCollection->NumberOfTimeGroups();
  for (int timeGroup = 0; timeGroup < NumTimeGroups; timeGroup++) {
    std::vector<DetectorHit_sptr>& hits = m_HitCollection->getHits(timeGroup);
    hits.erase(std::remove_if(hits.begin(), hits.end(),
                              [this](const DetectorHit_sptr& hit) {
                                if (m_PhotonThreshold < 0.0) {
                                  return hit->EPI() < m_EnergyThreshold;
                                }
                                return (hit->EPI() < m_EnergyThreshold) && hit->PhotonCount() < m_PhotonThreshold;
                              }),
               hits.end());
  }
  return AS_OK;
}
} // namespace comptonsoft