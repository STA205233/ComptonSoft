#include "ConstantRecombinationModel.hh"

namespace comptonsoft {
ConstantRecombinationModel::ConstantRecombinationModel()
  : VLArRecombinationModel(),
    rate_(1.0)
{
}

ConstantRecombinationModel::ConstantRecombinationModel(const std::string& name)
  : VLArRecombinationModel(name),
    rate_(1.0)
{
}
ConstantRecombinationModel::ConstantRecombinationModel(const std::string& name,
                                                       const std::map<std::string, double>& params)
  : VLArRecombinationModel(name, params)
{
  auto it = params.find("rate");
  if (it != params.end()) {
    rate_ = it->second;
  }
}
ConstantRecombinationModel::~ConstantRecombinationModel() = default;
double ConstantRecombinationModel::recombinationRate(double /*dedx*/, double /*electricField*/) const
{
  return rate_;
}
} // namespace comptonsoft