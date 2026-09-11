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

#include "GainFunctionTF1.hh"
namespace comptonsoft {
GainFunctionTF1::GainFunctionTF1(): func_(nullptr) {}
GainFunctionTF1::~GainFunctionTF1() = default;

double GainFunctionTF1::eval(double x) const {
  return func_->Eval(x);
}

double GainFunctionTF1::RangeMin() const {
  return func_->GetXmin();
}

double GainFunctionTF1::RangeMax() const {
  return func_->GetXmax();
}
} // namespace comptonsoft