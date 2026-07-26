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

#include "NanoGRAMSHistogram2D.hh"

#include "NanoGRAMSHistProperty.hh"

#include <TH2D.h>

#include <iostream>

using namespace anlnext;

namespace comptonsoft
{

NanoGRAMSHistogram2D::NanoGRAMSHistogram2D() = default;

NanoGRAMSHistogram2D::~NanoGRAMSHistogram2D() = default;

ANLStatus NanoGRAMSHistogram2D::mod_define()
{
  define_parameter("x_source_module_name", &mod_class::xSourceModuleName_);
  define_parameter("y_source_module_name", &mod_class::ySourceModuleName_);
  define_parameter("hist_name",            &mod_class::histName_);
  define_parameter("hist_title",           &mod_class::histTitle_);
  define_parameter("number_of_bins_x",     &mod_class::numBinsX_);
  define_parameter("x_min",                &mod_class::xMin_);
  define_parameter("x_max",                &mod_class::xMax_);
  define_parameter("number_of_bins_y",     &mod_class::numBinsY_);
  define_parameter("y_min",                &mod_class::yMin_);
  define_parameter("y_max",                &mod_class::yMax_);
  return AS_OK;
}

ANLStatus NanoGRAMSHistogram2D::mod_initialize()
{
  const ANLStatus status = VCSModule::mod_initialize();
  if (status != AS_OK) {
    return status;
  }

  get_module_IF(xSourceModuleName_, &xSource_);
  if (!xSource_) {
    std::cerr << "NanoGRAMSHistogram2D::mod_initialize: "
              << xSourceModuleName_
              << " does not provide VNanoGRAMSHistProperty<double>" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  get_module_IF(ySourceModuleName_, &ySource_);
  if (!ySource_) {
    std::cerr << "NanoGRAMSHistogram2D::mod_initialize: "
              << ySourceModuleName_
              << " does not provide VNanoGRAMSHistProperty<double>" << std::endl;
    return AS_QUIT_ALL_ERROR;
  }

  mkdir();
  hist_ = new TH2D(histName_.c_str(), histTitle_.c_str(),
                   numBinsX_, xMin_, xMax_,
                   numBinsY_, yMin_, yMax_);
  hist_->SetXTitle(xSource_->ValueName().c_str());
  hist_->SetYTitle(ySource_->ValueName().c_str());
  return AS_OK;
}

ANLStatus NanoGRAMSHistogram2D::mod_analyze()
{
  if (xSource_->Valid() && ySource_->Valid()) {
    hist_->Fill(xSource_->HistValue(), ySource_->HistValue());
  }
  return AS_OK;
}

} /* namespace comptonsoft */
