#ifndef COMPTONSOFT_ApplyCCE_hh
#define COMPTONSOFT_ApplyCCE_hh 1
#include "VCSModule.hh"
namespace comptonsoft {
class CSHitCollection;

/*
 * @brief Apply Charge collection efficiency (CCE) to hits
 * @author Shota Arai
 */
class ApplyCCE : public VCSModule
{
  DEFINE_ANL_MODULE(ApplyCCE, 1.0)
public:
  ApplyCCE();
  virtual ~ApplyCCE();

public:
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

private:
  CSHitCollection* hit_collection_;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_ApplyCCE_hh