#include "NestWrapper.hh"
using namespace NEST;
namespace comptonsoft {
LArNestModel::LArNestModel() : VLArRecombinationModel("LArNestModel") {}
LArNestModel::LArNestModel(const std::string &name, const std::map<std::string, double> &params)
    : VLArRecombinationModel(name, params) {}
LArNestModel::LArNestModel(const std::string &name) : VLArRecombinationModel(name) {}
LArNestModel::~LArNestModel() = default;
double LArNestModel::electronLet(double let, double electricField) const {
  if (let == lastLet_ && electricField == lastElectricField_) {
    return lastResult_.nIon * Wion();
  }
  lastLet_ = let;
  lastElectricField_ = electricField;
  lastResult_ = NEST::LArNEST::FullCalculation(NEST::LArInteraction::dEdx, let, electricField);
  return lastResult_.nIon * Wion();
}
double LArNestModel::lightYieldPerLength(double let, double electricField) const {
  if (let == lastLet_ && electricField == lastElectricField_) {
    return lastResult_.nPh * Wexc();
  }
  lastLet_ = let;
  lastElectricField_ = electricField;
  lastResult_ = NEST::LArNEST::FullCalculation(NEST::LArInteraction::dEdx, let, electricField);
  
  return lastResult_.nPh * Wexc();
}
void LArNestModel::printInfo(std::ostream &os) const {
  os << "Recombination Model: " << name() << std::endl;
}
} // namespace comptonsoft