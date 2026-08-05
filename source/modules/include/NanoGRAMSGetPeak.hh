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

#ifndef COMPTONSOFT_NanoGRAMSGetPeak_hh
#define COMPTONSOFT_NanoGRAMSGetPeak_hh 1
#include "CSHitCollection.hh"
#include "NanoGRAMSHistProperty.hh"
#include "NanoGRAMSLightWaveformQuery.hh"
namespace comptonsoft {

class NanoGRAMSGetPeak final : public VNanoGRAMSLightWaveformQuery, public VNanoGRAMSHistProperty<double>
{
  DEFINE_ANL_MODULE(NanoGRAMSGetPeak, 1.0)
public:
  NanoGRAMSGetPeak();
  virtual ~NanoGRAMSGetPeak();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_initialize() override;

  const auto& Peaks() const { return peaks_; }
  const auto& PeakPos() const { return peak_pos_; }

private:
  double x_min_ = 0.0 * CLHEP::us;
  double x_max_ = 5.0 * CLHEP::us;
  bool set_to_hit_ = false;
  std::vector<int> channel_list_;
  std::vector<double> peaks_;
  std::vector<double> peak_pos_;
  std::vector<bool> general_channel_;
  CSHitCollection* hit_collection_ = nullptr;

  void find_peak();
  void apply_to_hits();
};

} // namespace comptonsoft
#endif // COMPTONSOFT_NanoGRAMSGetPeak_hh