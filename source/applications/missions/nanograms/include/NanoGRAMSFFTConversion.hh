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

/**
 * @file NanoGRAMSFFTConversion.hh
 * @brief FFT/inverse-FFT conversion of a waveform histogram, ported from
 *        nanograms-analysis (light/core/include/FFTConversion.hh).
 * @author Shota Arai
 */

#ifndef COMPTONSOFT_NanoGRAMSFFTConversion_H
#define COMPTONSOFT_NanoGRAMSFFTConversion_H 1

#include <memory>

#include "TH1D.h"
#include "TVirtualFFT.h"

namespace comptonsoft
{
namespace grams {

class NanoGRAMSFFTConversion
{
public:
  NanoGRAMSFFTConversion() = default;
  virtual ~NanoGRAMSFFTConversion();

  TH1D* GetHist() const { return hist_; }
  std::shared_ptr<TH1D> GetHistFFT() const;
  std::shared_ptr<TH1D> GetHistBack();
  std::shared_ptr<TVirtualFFT> GetFFT() { return fft_; }
  std::shared_ptr<TVirtualFFT> GetFFTInverse() { return fftInverse_; }

  void SetHist(TH1D* hist);
  void ExecFFT();
  void ExecFFTInverse();
  double GetFrequency(double xPosition) const;
  double GetRangeX() const { return rangeX_; }

private:
  TH1D* hist_ = nullptr;
  bool calcHistBack_ = false;
  std::shared_ptr<TH1D> histFFT_ = nullptr;
  std::shared_ptr<TH1D> histBack_ = nullptr;
  std::shared_ptr<TVirtualFFT> fft_ = nullptr;
  std::shared_ptr<TVirtualFFT> fftInverse_ = nullptr;
  double rangeX_ = 0;
  bool FFTExecuted_ = false;
};

} /* namespace grams */
} /* namespace comptonsoft */

#endif /* COMPTONSOFT_NanoGRAMSFFTConversion_H */
