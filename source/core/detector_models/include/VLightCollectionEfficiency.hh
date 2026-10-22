#ifndef COMPTONSOFT_VLightCollectionEfficiency_hh
#define COMPTONSOFT_VLightCollectionEfficiency_hh 1
namespace comptonsoft {
class VLightCollectionEfficiency
{
public:
  VLightCollectionEfficiency() = default;
  virtual ~VLightCollectionEfficiency() = default;

public:
  virtual double Efficiency(double x, double y, double z) const = 0;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_VLightCollectionEfficiency_hh