#ifndef COMPTONSOFT_PushBackToSimulatedHits_hh
#define COMPTONSOFT_PushBackToSimulatedHits_hh 1
#include "VCSModule.hh"
namespace comptonsoft {
class CSHitCollection;
class PushBackToSimulatedHits : public VCSModule
{
  DEFINE_ANL_MODULE(PushBackToSimulatedHits, 1.0)
public:
  PushBackToSimulatedHits();
  virtual ~PushBackToSimulatedHits();
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

private:
  CSHitCollection* hit_collection_;
};
}; // namespace comptonsoft
#endif // COMPTONSOFT_PushBackToSimulatedHits_hh