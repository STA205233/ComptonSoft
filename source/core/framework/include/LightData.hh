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

#ifndef COMPTONSOFT_LightData_hh
#define COMPTONSOFT_LightData_hh 1
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
namespace comptonsoft {
class VGainFunction;

/**
 * @brief A class of a light signal
 * @date 2026-09-06 | Shota Arai | First version
 */
class LightData
{
public:
  using range_t = std::pair<double, double>;
  using range_index_t = std::pair<int, int>;
  LightData(int n_points, double bin_width, double t_start);
  virtual ~LightData();

private:
  LightData& operator=(const LightData& r) = delete;
  LightData& operator=(LightData&& r) = delete;

private:
  double pedestal_ = 0.0;
  bool valid_ = true;

  int n_points_ = 1;
  double t_width_ = 0;
  double t_start_ = 0;

protected:
  double correctGainAtEachBin(double x) const;
  double mean(const range_index_t& range) const;
  double integral(const range_index_t& range) const;
  double peakPosition(const range_index_t& range) const;
  double peak(const range_index_t& range) const;
  range_index_t convertRange(const range_t& range) const;

public:
  std::vector<double>& Waveform() { return waveform_; }
  const std::vector<double>& Waveform() const { return waveform_; }
  // waveform as read from the data (before gain correction)
  std::vector<double>& RawWaveform() { return raw_waveform_; }
  const std::vector<double>& RawWaveform() const { return raw_waveform_; }
  const std::vector<double>& Time() const { return time_; }

  int NumberOfPoints() const { return n_points_; }
  double TimeWidth() const { return t_width_; }
  double TimeStart() const { return t_start_; }

  // false if the channel has no waveform (e.g., not recorded or not used)
  void setValid(bool v) { valid_ = v; }
  bool isValid() const { return valid_; }
  /**
   * change the number of points, the time width, and the start time.
   * The waveforms are resized and reset.
   */
  void setLayout(int n_points, double t_width, double t_start);
  void resetTimeStart(double t_start);
  void correctGain();
  void setPedestal(double pedestal) { pedestal_ = pedestal; }
  double pedestal() const { return pedestal_; }
  virtual void subtractPedestal();

  double integral(const range_t& range) const;
  double mean(const range_t& range) const;
  double peakPotision(const range_t& range) const;
  double peak(const range_t& range) const;
  int findTimeIndex(double time) const;

  void setGainFunction(const std::shared_ptr<VGainFunction>& f) { gain_function_ = f; }
  const VGainFunction* getGainFunction() const { return gain_function_.get(); }
  void resetGainFunction() { gain_function_.reset(); }

  // reset functions
  void resetEventData();
  void resetWaveform() { std::fill(waveform_.begin(), waveform_.end(), 0); }
  void resetRawWaveform() { std::fill(raw_waveform_.begin(), raw_waveform_.end(), 0); }

private:
  std::vector<double> waveform_;
  std::vector<double> raw_waveform_;
  std::vector<double> time_;
  std::shared_ptr<VGainFunction> gain_function_;

  double sum(const range_index_t& range) const;
  range_index_t clampRange(const range_index_t& range) const;
};
} // namespace comptonsoft
#endif