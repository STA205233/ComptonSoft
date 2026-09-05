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

#ifndef COMPTONSOFT_LightWaveform_hh
#define COMPTONSOFT_LightWaveform_hh 1
#include <map>
#include <vector>
namespace comptonsoft {
class LightData
{
  using value_t = double;
  LightData();
  virtual ~LightData();

public:
  std::vector<value_t>& waveform(int channel)
  {
    static std::vector<value_t> empty(0);
    const auto iter = waveforms_.find(channel);
    return iter != waveforms_.end() ? iter->second : empty;
  }

private:
  std::map<int, std::vector<value_t>> waveforms_; // ch, vector
};
} // namespace comptonsoft
#endif