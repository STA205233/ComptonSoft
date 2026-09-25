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


#ifndef COMPTONSOFT_NanoGRAMSSimpleFilter_H
#define COMPTONSOFT_NanoGRAMSSimpleFilter_H 1

#include <iostream>

#include "NanoGRAMSFFTFilterImpl.hh"

namespace comptonsoft
{
namespace grams
{

struct SimpleFilterParam
{
  double lowFrequency = 0.0;
  double highFrequency = 0.0;
};

using SimpleFilter = FFTFilterImpl<SimpleFilterParam>;

template <>
inline void FFTFilterImpl<SimpleFilterParam>::ApplyFilter()
{
  auto histFFT = fftConversion_->GetHistFFT();
  auto fft = fftConversion_->GetFFT();
  auto inverseFFT = fftConversion_->GetFFTInverse();
  if (!histFFT) {
    std::cerr << "FFT has not been executed" << std::endl;
    return;
  }
  const int nBins = histFFT->GetNbinsX();
  for (int ibin = 1; ibin <= nBins; ++ibin) {
    const double frequency = fftConversion_->GetFrequency(histFFT->GetBinCenter(ibin));
    double im = 0.0;
    double re = 0.0;
    fft->GetPointComplex(ibin - 1, re, im);
    if (frequency < param_.lowFrequency || frequency > param_.highFrequency) {
      inverseFFT->SetPoint(ibin - 1, 0.0, 0.0);
    }
    else {
      inverseFFT->SetPoint(ibin - 1, re / static_cast<double>(nBins), im / static_cast<double>(nBins));
    }
  }
}

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSSimpleFilter_H */
