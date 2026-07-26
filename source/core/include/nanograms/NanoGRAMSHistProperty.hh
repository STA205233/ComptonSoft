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

#ifndef COMPTONSOFT_NanoGRAMSHistProperty_hh
#define COMPTONSOFT_NanoGRAMSHistProperty_hh 1
#include <string>

namespace comptonsoft {
template <typename T>
class VNanoGRAMSHistProperty
{

public:
  VNanoGRAMSHistProperty() = default;
  virtual ~VNanoGRAMSHistProperty() = default;
  
protected:
  void setHistValue(T value)
  {
    value_ = value;
    valid_ = true;
  }
  void setHistValueName(const std::string& name) { name_ = name; }
  void resetHistValue()
  {
    value_ = T();
    valid_ = false;
  }

public:
  T HistValue() const { return value_; }
  const std::string& ValueName() const { return name_; }
  const T& HistValueRef() const { return value_; }
  bool Valid() const { return valid_; }
private:
  T value_;
  std::string name_;
  bool valid_ = true;
};
}
#endif /* COMPTONSOFT_NanoGRAMSHistProperty_hh */