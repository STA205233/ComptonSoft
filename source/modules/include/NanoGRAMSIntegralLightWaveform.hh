/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#ifndef COMPTONSOFT_NanoGRAMSIntegralLightWaveform_hh
#define COMPTONSOFT_NanoGRAMSIntegralLightWaveform_hh 1

#include "NanoGRAMSHistProperty.hh"
#include "NanoGRAMSLightWaveformQuery.hh"
#include <vector>
class TH1D;

namespace comptonsoft {

class CSHitCollection;

class NanoGRAMSIntegralLightWaveform : public VNanoGRAMSLightWaveformQuery, public VNanoGRAMSHistProperty<double>
{
  DEFINE_ANL_MODULE(NanoGRAMSIntegralLightWaveform, "1.0")

public:
  NanoGRAMSIntegralLightWaveform();
  virtual ~NanoGRAMSIntegralLightWaveform();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_initialize() override;

  /**
   * @return the analysis channels, fixed after mod_initialize().
   */
  const std::vector<int>& Channels() const { return channel_list_; }

  /**
   * @return the integral value of each channel in Channels().
   * The value is NaN if the waveform of the channel is not available.
   */
  const std::vector<double>& Integrals() const { return integrals_; }

private:
  double xMin_ = 0.0 * CLHEP::us;
  double xMax_ = 5.0 * CLHEP::us;
  bool set_to_hit_ = false;

  std::vector<int> channel_list_;
  std::vector<double> integrals_;
  std::vector<bool> general_channel_;
  std::vector<double> coeffs_;
  std::string unit_ = "mV us";
  CSHitCollection* hit_collection_ = nullptr;

  void integrate();
  void applyToHits();
};

} // namespace comptonsoft

#endif /* COMPTONSOFT_NanoGRAMSIntegralLightWaveform_hh */