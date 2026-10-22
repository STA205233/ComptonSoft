#ifndef COMPTONSOFT_EliminateLowEnergyHits_hh
#define COMPTONSOFT_EliminateLowEnergyHits_hh 1
#include "CLHEP/Units/SystemOfUnits.h"
#include "CSHitCollection.hh"
#include "anlnext/BasicModule.hh"

namespace comptonsoft {
class EliminateLowEnergyHits : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(EliminateLowEnergyHits, 1.0);
  ENABLE_PARALLEL_RUN();

public:
  EliminateLowEnergyHits();
  virtual ~EliminateLowEnergyHits();

protected:
  EliminateLowEnergyHits(const EliminateLowEnergyHits& r);

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

private:
  CSHitCollection* m_HitCollection = nullptr;
  double m_EnergyThreshold = 0.0;
  double m_PhotonThreshold = -std::numeric_limits<double>::infinity();
};
} // namespace comptonsoft
#endif // COMPTONSOFT_EliminateLowEnergyHits_hh