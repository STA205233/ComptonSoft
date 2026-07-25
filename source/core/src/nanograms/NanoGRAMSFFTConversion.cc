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

#include "NanoGRAMSFFTConversion.hh"

#include <iostream>

namespace comptonsoft
{
namespace grams
{

NanoGRAMSFFTConversion::~NanoGRAMSFFTConversion()
{
  fft_.reset();
  fftInverse_.reset();
}

std::shared_ptr<TH1D> NanoGRAMSFFTConversion::GetHistFFT() const
{
  if (!FFTExecuted_) {
    std::cerr << "FFT has not been executed" << std::endl;
    return nullptr;
  }
  return histFFT_;
}

std::shared_ptr<TH1D> NanoGRAMSFFTConversion::GetHistBack()
{
  if (!calcHistBack_) {
    std::cerr << "histBack is not calculated" << std::endl;
    return nullptr;
  }
  calcHistBack_ = false;
  return histBack_;
}

void NanoGRAMSFFTConversion::SetHist(TH1D* hist)
{
  hist_ = hist;
  FFTExecuted_ = false;
  histFFT_.reset();
  histBack_.reset();
}

void NanoGRAMSFFTConversion::ExecFFT()
{
  if (!hist_) {
    std::cerr << "histogram is not set" << std::endl;
    return;
  }
  TVirtualFFT::SetTransform(nullptr);
  histFFT_ = std::shared_ptr<TH1D>(static_cast<TH1D*>(hist_->FFT(histFFT_.get(), "MAG")));
  histFFT_->SetNameTitle("histFFT", "histFFT");
  histFFT_->SetDirectory(nullptr); // Prevent ROOT from deleting the histogram
  int n = hist_->GetNbinsX();
  fft_.reset(TVirtualFFT::GetCurrentTransform());
  if (!fftInverse_ || fftInverse_->GetN()[0] != n) {
    fftInverse_.reset(TVirtualFFT::FFT(1, &n, "C2R M K"));
  }
  rangeX_ = hist_->GetBinCenter(n) - hist_->GetBinCenter(1);
  FFTExecuted_ = true;
}

void NanoGRAMSFFTConversion::ExecFFTInverse()
{
  if (!FFTExecuted_) {
    std::cerr << "FFT may not be executed" << std::endl;
    return;
  }
  fftInverse_->Transform();
  histBack_ = std::shared_ptr<TH1D>(
      static_cast<TH1D*>(TH1::TransformHisto(fftInverse_.get(), histBack_.get(), "RE")));
  histBack_->SetNameTitle("histBack", "histBack");
  histBack_->SetDirectory(nullptr); // Prevent ROOT from deleting the histogram
  FFTExecuted_ = false;
  calcHistBack_ = true;
}

double NanoGRAMSFFTConversion::GetFrequency(double xPosition) const
{
  if (rangeX_ == 0) {
    return -1.0;
  }
  return xPosition / rangeX_;
}

} /* namespace grams */
} /* namespace comptonsoft */
