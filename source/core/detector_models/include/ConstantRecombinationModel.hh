#ifndef COMPTONSOFT_ConstantRecombinationModel_hh
#define COMPTONSOFT_ConstantRecombinationModel_hh 1
#include "VLArRecombinationModel.hh"
namespace comptonsoft {

class ConstantRecombinationModel : public VLArRecombinationModel
{
public:
  ConstantRecombinationModel();
  ConstantRecombinationModel(const std::string& name, const std::map<std::string, double>& params);
  ConstantRecombinationModel(const std::string& name);
  virtual ~ConstantRecombinationModel();

  double recombinationRate(double dedx, double electricField) const override;

private:
  double rate_;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_ConstantRecombinationModel_hh