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

#include "FillFrame.hh"

#include "AstroUnits.hh"
#include "CSHitCollection.hh"
#include "DetectorHit.hh"
#include "ConstructFrame.hh"
#include "FrameData.hh"


using namespace anlnext;
namespace unit = anlgeant4::unit;

namespace comptonsoft
{

FillFrame::FillFrame()
{
}

FillFrame::~FillFrame() = default;

ANLStatus FillFrame::mod_define()
{
  return AS_OK;
}

ANLStatus FillFrame::mod_initialize()
{
  VCSModule::mod_initialize();
  get_module_NC("CSHitCollection", &m_HitCollection);
  get_module_NC("ConstructFrame", &m_FrameOwner);

  return AS_OK;
}

ANLStatus FillFrame::mod_analyze()
{
  const std::vector<DetectorHit_sptr>& firstHits = m_HitCollection->getHits(0);
  if (firstHits.size() == 0) { return AS_OK; }

  const DetectorHit_sptr& hit0 = firstHits[0];
  const long frameID = hit0->EventID();

  m_FrameOwner->setFrameID(frameID);
  FrameData& frame = m_FrameOwner->getFrame();
  frame.resetRawFrame();
  const int nx = frame.NumPixelsX();
  const int ny = frame.NumPixelsY();
  image_t& rawFrame = frame.getRawFrame();

  const int NumTimeGroups = m_HitCollection->NumberOfTimeGroups();
  for (int timeGroup=0; timeGroup<NumTimeGroups; timeGroup++) {
    const std::vector<DetectorHit_sptr>& hits = m_HitCollection->getHits(timeGroup);
    for (DetectorHit_sptr hit: hits) {
      const int ix = hit->PixelX();
      const int iy = hit->PixelY();
      if (ix >= nx || iy >= ny) {
        std::cout << "This hit occurs outside the image area." << std::endl;
        return AS_ERROR;
      }

      rawFrame[ix][iy] += hit->EPI()/unit::keV;
    }
  }
  
  return AS_OK;
}

} /* namespace comptonsoft */