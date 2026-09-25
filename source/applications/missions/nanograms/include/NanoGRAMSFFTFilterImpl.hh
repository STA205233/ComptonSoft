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


#ifndef COMPTONSOFT_NanoGRAMSFFTFilterImpl_H
#define COMPTONSOFT_NanoGRAMSFFTFilterImpl_H 1

#include "NanoGRAMSFFTConversion.hh"
#include "NanoGRAMSVLightWaveformFilter.hh"

namespace comptonsoft
{
namespace grams
{

template <typename ParamType>
class FFTFilterImpl: public VLightWaveformFilter
{
public:
  FFTFilterImpl() = default;
  virtual ~FFTFilterImpl() = default;
  FFTFilterImpl(const FFTFilterImpl& r)
    : VLightWaveformFilter(r), fftConversion_(nullptr), param_(r.param_) {}

  std::shared_ptr<TH1D> Exec(std::shared_ptr<TH1D> signal_hist) override;
  void SetParam(const ParamType& param) { param_ = param; }

private:
  void ApplyFilter();

  std::unique_ptr<NanoGRAMSFFTConversion> fftConversion_ = nullptr;
  ParamType param_;
};

template <typename ParamType>
std::shared_ptr<TH1D> FFTFilterImpl<ParamType>::Exec(std::shared_ptr<TH1D> signal_hist)
{
  if (!fftConversion_) {
    fftConversion_ = std::make_unique<NanoGRAMSFFTConversion>();
  }
  fftConversion_->SetHist(signal_hist.get());
  fftConversion_->ExecFFT();
  ApplyFilter();
  fftConversion_->ExecFFTInverse();
  return fftConversion_->GetHistBack();
}

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSFFTFilterImpl_H */
