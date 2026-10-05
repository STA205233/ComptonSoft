#ifndef COMPTONSOFT_NanoGRAMSLoadConfig_hh
#define COMPTONSOFT_NanoGRAMSLoadConfig_hh 1
#include "NanoGRAMSCalibrationData.hh"
#include "NanoGRAMSConfig.hh"
#include "VCSModule.hh"

namespace comptonsoft {

class NanoGRAMSLoadConfig : public VCSModule
{
  DEFINE_ANL_MODULE(NanoGRAMSLoadConfig, 1.0);

public:
  NanoGRAMSLoadConfig() = default;
  virtual ~NanoGRAMSLoadConfig() = default;

protected:
  NanoGRAMSLoadConfig(const NanoGRAMSLoadConfig& r) = default;

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  const grams::Config& config() const { return cfg_; }
  grams::Config& config() { return cfg_; }
  const std::string& configFilePath() const { return config_file_; }
  const std::string& dppConfigFilePath() const { return dpp_config_file_; }

  bool IsDppConfigSet() const { return !dpp_config_file_.empty(); }
  bool IsConfigSet() const { return !config_file_.empty(); }

  void setupCalibration();
  void setupDetectorParameters(const std::array<GainMatrix, NUM_VATA>& adc2c,
                               const std::shared_ptr<const NanoGRAMSTemperatureCorrection>& temperatureCorrection);

private:
  std::string config_file_;
  std::string dpp_config_file_ = "";
  grams::Config cfg_;
  CalibrationConfig calibration_config_;
  std::string gain_tp_file_ = "";
  std::map<std::string, double> gain_tp_dict_;
  double gain_tp_value_ = 0.0;
};
} // namespace comptonsoft
#endif // COMPTONSOFT_NanoGRAMSLoadConfig_hh