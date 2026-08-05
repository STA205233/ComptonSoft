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

#ifndef COMPTONSOFT_WriteNanoGRAMSLightTree_H
#define COMPTONSOFT_WriteNanoGRAMSLightTree_H 1

#include "VCSModule.hh"
#include <memory>
#include <string>

class TTree;

namespace comptonsoft {

class NanoGRAMSIntegralLightWaveform;
class NanoGRAMSLightTreeIO;
class NanoGRAMSGetPeak;

/**
 * Write the light waveform integrals of NanoGRAMSIntegralLightWaveform
 * into a tree. One event makes one entry so that the tree can be used as
 * a friend tree of the event tree.
 *
 * @author Shota Arai
 * @date 2026-07-29
 */
class WriteNanoGRAMSLightTree : public VCSModule
{
  DEFINE_ANL_MODULE(WriteNanoGRAMSLightTree, 1.0);
public:
  WriteNanoGRAMSLightTree();
  ~WriteNanoGRAMSLightTree();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

private:
  std::string integralModule_;
  std::string peakModule_;
  const NanoGRAMSIntegralLightWaveform* integral_ = nullptr;
  const NanoGRAMSGetPeak* peak_ = nullptr;
  TTree* tree_ = nullptr;
  std::unique_ptr<NanoGRAMSLightTreeIO> treeIO_;
};

} /* namespace comptonsoft */

#endif /* COMPTONSOFT_WriteNanoGRAMSLightTree_H */
