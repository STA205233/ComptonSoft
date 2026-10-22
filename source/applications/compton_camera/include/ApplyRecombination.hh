#ifndef COMPTONSOFT_SetKineticEnergy_hh
#define COMPTONSOFT_SetKineticEnergy_hh 1
#include "VCSModule.hh"
#include <string>
#include <string_view>
namespace comptonsoft {
class DetectorHit;
class CSHitCollection;

/*
 * @brief Apply Recombination to hits. This module does not use rawhit.
 * @author Shota Arai
 */
class ApplyRecombination : public VCSModule
{
  DEFINE_ANL_MODULE(ApplyRecombination, 1.0)
public:
  ApplyRecombination();
  virtual ~ApplyRecombination();

  enum class SourceType
  {
    EDEP,
    ECHARGE,
    EPI,
  };

private:
  std::string source_str_;
  SourceType source_;
  CSHitCollection* hit_collection_;

  static SourceType parseSourceType(std::string_view str);
  using getter_func_ptr = double (DetectorHit::*)() const;
  static getter_func_ptr getGetter(SourceType sourceType);

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_SetKineticEnergy_hh