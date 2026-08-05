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

#include "WriteNanoGRAMSLightTree.hh"

#include "NanoGRAMSGetPeak.hh"
#include "NanoGRAMSIntegralLightWaveform.hh"
#include "NanoGRAMSLightTreeIO.hh"
#include "TTree.h"

using namespace anlnext;

namespace comptonsoft {

WriteNanoGRAMSLightTree::WriteNanoGRAMSLightTree()
  : integralModule_("NanoGRAMSIntegralLightWaveform"),
    peakModule_("NanoGRAMSGetPeak"),
    treeIO_(new NanoGRAMSLightTreeIO)
{
}

WriteNanoGRAMSLightTree::~WriteNanoGRAMSLightTree() = default;

ANLStatus WriteNanoGRAMSLightTree::mod_define()
{
  define_parameter("integral_module", &mod_class::integralModule_);
  define_parameter("peak_module", &mod_class::peakModule_);

  return AS_OK;
}

ANLStatus WriteNanoGRAMSLightTree::mod_initialize()
{
  VCSModule::mod_initialize();

  if (!exist_module(integralModule_)) {
    std::cerr << module_id() << ": module " << integralModule_
              << " does not exist in the same ANL chain." << std::endl;
    return AS_QUIT_ALL_ERROR;
  }
  get_module(integralModule_, &integral_);

  if (!exist_module(peakModule_)) {
    std::cerr << module_id() << ": module " << peakModule_
              << " does not exist in the same ANL chain." << std::endl;
    return AS_QUIT_ALL_ERROR;
  }
  get_module(peakModule_, &peak_);

  tree_ = new TTree("lighttree", "lighttree");
  treeIO_->setTree(tree_);
  treeIO_->defineBranches();

  return AS_OK;
}

ANLStatus WriteNanoGRAMSLightTree::mod_analyze()
{
  treeIO_->fillEvent(integral_->Channels(),
                     integral_->Integrals(),
                     peak_->Peaks(),
                     peak_->PeakPos(),
                     integral_->HistValue(),
                     peak_->HistValue());

  return AS_OK;
}

} /* namespace comptonsoft */
