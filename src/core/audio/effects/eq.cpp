/*
    MIT License

    Copyright (c) 2025 Evandro

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#include "eq.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PCore {

    void Eq::Biquad::processBlock(const float* in, float* out, unsigned long frames) {
        for (unsigned long i = 0; i < frames; ++i) {
            float x = in[i];
            float y = b0 * x + b1 * z1 + b2 * z2 - a1 * z1 - a2 * z2;
            z2 = z1;
            z1 = y; // Direct Form I transposed or similar structure? 
            // Wait, standard DF1: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
            // The code above looks like DF2 or TDF2 if we delay nodes correctly.
            // Let's stick to standard DF1 implementation for clarity, but memory handling is trickier.
            // Let's use Transposed Direct Form II for stability with floating point
            // y[n] = b0*x[n] + z1;
            // z1 = b1*x[n] - a1*y[n] + z2;
            // z2 = b2*x[n] - a2*y[n];
            
            // Re-implementing TDF2
            float outSample = b0 * x + z1;
            z1 = b1 * x - a1 * outSample + z2;
            z2 = b2 * x - a2 * outSample;
            
            // Avoid denormals
            if (std::fabs(z1) < 1e-20f) z1 = 0.0f;
            if (std::fabs(z2) < 1e-20f) z2 = 0.0f;

            out[i] = outSample; // Can be in-place
        }
    }

    Eq::Eq(int sampleRate) : sampleRate_(sampleRate) {
        recalculateCoefficients();
    }

    void Eq::prepare(int sampleRate, int maxBlock, int inChans, int outChans) {
        sampleRate_ = sampleRate;
        size_t channels = std::max(inChans, outChans);
        
        filters_.resize(channels);
        for (auto& chFilters : filters_) {
            chFilters.resize(3); // Low, Mid, High
            for (auto& b : chFilters) b.reset();
        }
        recalculateCoefficients();
    }

    void Eq::calculateLowShelf(Biquad& f, float freq, float gainDB) {
        float A = std::pow(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / sampleRate_;
        float alpha = std::sin(w0) / 2.0f * std::sqrt((A + 1.0f/A)*(1.0f/0.707f - 1.0f) + 2.0f);
        float cw0 = std::cos(w0);

        float b0 =    A*( (A+1.0f) - (A-1.0f)*cw0 + 2.0f*std::sqrt(A)*alpha );
        float b1 = 2.0f*A*( (A-1.0f) - (A+1.0f)*cw0                   );
        float b2 =    A*( (A+1.0f) - (A-1.0f)*cw0 - 2.0f*std::sqrt(A)*alpha );
        float a0 =        (A+1.0f) + (A-1.0f)*cw0 + 2.0f*std::sqrt(A)*alpha;
        float a1 = -2.0f*( (A-1.0f) + (A+1.0f)*cw0                   );
        float a2 =        (A+1.0f) + (A-1.0f)*cw0 - 2.0f*std::sqrt(A)*alpha;

        f.b0 = b0 / a0;
        f.b1 = b1 / a0;
        f.b2 = b2 / a0;
        f.a1 = a1 / a0;
        f.a2 = a2 / a0;
    }

    void Eq::calculatePeaking(Biquad& f, float freq, float Q, float gainDB) {
        float A = std::pow(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / sampleRate_;
        float alpha = std::sin(w0) / (2.0f * Q);
        float cw0 = std::cos(w0);

        float b0 =   1.0f + alpha * A;
        float b1 =  -2.0f * cw0;
        float b2 =   1.0f - alpha * A;
        float a0 =   1.0f + alpha / A;
        float a1 =  -2.0f * cw0;
        float a2 =   1.0f - alpha / A;

        f.b0 = b0 / a0;
        f.b1 = b1 / a0;
        f.b2 = b2 / a0;
        f.a1 = a1 / a0;
        f.a2 = a2 / a0;
    }

    void Eq::calculateHighShelf(Biquad& f, float freq, float gainDB) {
        float A = std::pow(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / sampleRate_;
        float alpha = std::sin(w0) / 2.0f * std::sqrt((A + 1.0f/A)*(1.0f/0.707f - 1.0f) + 2.0f);
        float cw0 = std::cos(w0);

        float b0 =    A*( (A+1.0f) + (A-1.0f)*cw0 + 2.0f*std::sqrt(A)*alpha );
        float b1 = -2.0f*A*( (A-1.0f) + (A+1.0f)*cw0                   );
        float b2 =    A*( (A+1.0f) + (A-1.0f)*cw0 - 2.0f*std::sqrt(A)*alpha );
        float a0 =        (A+1.0f) - (A-1.0f)*cw0 + 2.0f*std::sqrt(A)*alpha;
        float a1 =  2.0f*( (A-1.0f) - (A+1.0f)*cw0                   );
        float a2 =        (A+1.0f) - (A-1.0f)*cw0 - 2.0f*std::sqrt(A)*alpha;

        f.b0 = b0 / a0;
        f.b1 = b1 / a0;
        f.b2 = b2 / a0;
        f.a1 = a1 / a0;
        f.a2 = a2 / a0;
    }

    void Eq::recalculateCoefficients() {
        if (sampleRate_ <= 0) return;

        for (auto& chFilters : filters_) {
            if (chFilters.size() >= 3) {
                calculateLowShelf(chFilters[0], lowFreq_, lowGain_);
                calculatePeaking(chFilters[1], midFreq_, midQ_, midGain_);
                calculateHighShelf(chFilters[2], highFreq_, highGain_);
            }
        }
    }

    void Eq::setParameters(const std::string& param, float value) {
        bool recalc = true;
        if (param == "low_gain") lowGain_ = value;
        else if (param == "mid_gain") midGain_ = value;
        else if (param == "high_gain") highGain_ = value;
        else if (param == "low_freq") lowFreq_ = value; // Check range?
        else if (param == "mid_freq") midFreq_ = value;
        else if (param == "high_freq") highFreq_ = value;
        else if (param == "mid_q") midQ_ = std::max(0.1f, value);
        else recalc = false;

        if (recalc) recalculateCoefficients();
    }

    void Eq::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;
        if (!in || !in[0]) {
            std::fill_n(out[0], frames, 0.0f);
            return;
        }

        size_t numChannels = filters_.size();

        for (size_t c = 0; c < numChannels; ++c) {
            if (!in[c] || !out[c]) break;
            
            // We can process in-place effectively if Eq is cascaded
            // Or we copy input to output and then process in-place on output
            std::copy_n(in[c], frames, out[c]);
            
            // Cascade filter sections
            for (auto& filter : filters_[c]) {
                filter.processBlock(out[c], out[c], frames);
            }
        }
    }
}
