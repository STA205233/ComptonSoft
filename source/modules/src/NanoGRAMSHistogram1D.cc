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

#include "NanoGRAMSHistogram1D.hh"

#include "NanoGRAMSHistProperty.hh"

#include <TH1D.h>

#include <iostream>

using namespace anlnext;

namespace comptonsoft
{

NanoGRAMSHistogram1D::NanoGRAMSHistogram1D() = default;

NanoGRAMSHistogram1D::~NanoGRAMSHistogram1D() = default;

ANLStatus NanoGRAMSHistogram1D::mod_define()
{
  define_parameter("source_module_name", &mod_class::sourceModuleName_);
  define_parameter("hist_name",          &mod_class::histName_);
  define_parameter("hist_title",         &mod_class::histTitle_);
  define_parameter("number_of_bins",     &mod_class::numBins_);
  define_parameter("x_min",              &mod_class::xMin_);
  define_parameter("x_max",              &mod_class::xMax_);
  return AS_OK;
}

ANLStatus NanoGRAMSHistogram1D::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  get_module_IF(sourceModuleName_, &source_);
  if (!source_) {
    std::cerr << "NanoGRAMSHistogram1D::mod_initialize: "
              << sourceModuleName_
              << " does not provide VNanoGRAMSHistProperty<double>" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  mkdir();
  hist_ = new TH1D(histName_.c_str(), histTitle_.c_str(), numBins_, xMin_, xMax_);

  return AS_OK;
}

ANLStatus NanoGRAMSHistogram1D::mod_analyze()
{
  hist_->Fill(source_->HistValue());
  return AS_OK;
}

} /* namespace comptonsoft */
