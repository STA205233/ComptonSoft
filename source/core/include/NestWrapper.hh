#ifndef COMPTONSOFT_NestWrapper_hh
#define COMPTONSOFT_NestWrapper_hh 1
#include "NEST/LArNEST.hh"
#include "VLArRecombinationModel.hh"
namespace comptonsoft {
class LArNestModel: public VLArRecombinationModel {
public:
  LArNestModel();
  LArNestModel(const std::string &name, const std::map<std::string, double> &params);
  LArNestModel(const std::string &name);
  ~LArNestModel() override;
  double electronLet(double let, double electricField) const override;
  double lightYieldPerLength(double let, double electricField) const override;
  void printInfo(std::ostream &os) const override;

private:
  double lastLet_ = 0;
  double lastElectricField_ = 0;
  NEST::LArNESTResults lastResult_;
};
} // namespace comptonsoft
#endif //COMPTONSOFT_NestWrapper_hh