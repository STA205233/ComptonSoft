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

#ifndef COMPTONSOFT_NanoGRAMSHistogram2D_hh
#define COMPTONSOFT_NanoGRAMSHistogram2D_hh 1

#include <string>

#include "VCSModule.hh"

class TH2D;

namespace comptonsoft
{

template <typename T> class VNanoGRAMSHistProperty;

class NanoGRAMSHistogram2D : public VCSModule
{
  DEFINE_ANL_MODULE(NanoGRAMSHistogram2D, 1.0);

public:
  NanoGRAMSHistogram2D();
  virtual ~NanoGRAMSHistogram2D() override;

  anlnext::ANLStatus mod_define()     override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze()    override;

private:
  std::string xSourceModuleName_ = "";
  std::string ySourceModuleName_ = "";
  std::string histName_ = "hist2d";
  std::string histTitle_ = "hist2d";
  int numBinsX_ = 100;
  double xMin_ = 0.0;
  double xMax_ = 1.0;
  int numBinsY_ = 100;
  double yMin_ = 0.0;
  double yMax_ = 1.0;

  const VNanoGRAMSHistProperty<double>* xSource_ = nullptr;
  const VNanoGRAMSHistProperty<double>* ySource_ = nullptr;
  TH2D* hist_ = nullptr;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSHistogram2D_hh */
