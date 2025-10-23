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

#include <cmath>
#include <algorithm>

#include "flanger.h"

namespace PCore {
    Flanger::Flanger(int sampleRate) : sampleRate_(sampleRate) {}

    void Flanger::prepare(int sr, int block, int inCh, int outCh) {
        sampleRate_ = sr;

        const size_t baseSmp  = secToSamples(baseDelaySec_);
        const size_t depthSmp = secToSamples(depthSec_);
        const size_t total    = std::max<size_t>(baseSmp + depthSmp + 8, 512);

        dBuffer_.assign(total, 0.0f);
        writePos_ = 0;
        lfoPhase_ = 0.0f;
    }

    void Flanger::process(const float* const* in, float* const* out, unsigned long frames) {
        if (!out || !out[0]) return;

        const float* x = (in && in[0]) ? in[0] : nullptr;
        float* y = out[0];

        if (!x) {
            std::fill_n(y, frames, 0.0f);
            return;
        }

        const size_t N = dBuffer_.size();
        if (N == 0) { std::copy(x, x + frames, y); return; }

        const float twoPi = 6.283185307179586f;
        const float phaseInc = twoPi * (lfoRate_ / static_cast<float>(sampleRate_));

        const float baseSmpF  = static_cast<float>(secToSamples(baseDelaySec_));
        const float depthSmpF = static_cast<float>(secToSamples(depthSec_));

        size_t wp = writePos_;
        float phase = lfoPhase_;

        for (unsigned long i = 0; i < frames; ++i) {
            const float inS = x[i];

            float lfo = std::sin(phase);
            float modDelay = baseSmpF + (1.0f + lfo) * 0.5f * (2.0f * depthSmpF); //  ±depth
       
            float rpF = static_cast<float>(wp) - modDelay;
            while (rpF < 0.0f) rpF += static_cast<float>(N);

            size_t rp0 = static_cast<size_t>(rpF);
            size_t rp1 = (rp0 + 1) % N;
            float frac = rpF - static_cast<float>(rp0);

            float d0 = dBuffer_[rp0];
            float d1 = dBuffer_[rp1];
            float delayed = d0 + (d1 - d0) * frac;

            float fbIn = inS + feedback_ * delayed;

            dBuffer_[wp] = fbIn;

            y[i] = (1.0f - mix_) * inS + mix_ * delayed;

            wp = (wp + 1) % N;
            phase += phaseInc;
            if (phase >= twoPi) phase -= twoPi;
        }

        writePos_ = wp;
        lfoPhase_ = phase;
    }

    void Flanger::setParameters(const std::string& param, float value) {
        if (param == "rate") {
            lfoRate_ = std::clamp(value, 0.05f, 5.0f);
        } else if (param == "depth") {
            depthSec_ = std::clamp(value, 0.0001f, 0.010f);

            const size_t baseSmp  = secToSamples(baseDelaySec_);
            const size_t depthSmp = secToSamples(depthSec_);
            const size_t total    = std::max<size_t>(baseSmp + depthSmp + 8, 512);
            if (dBuffer_.size() != total) {
                dBuffer_.assign(total, 0.0f);
                writePos_ = 0;
            }
        } else if (param == "feedback") {
            feedback_ = std::clamp(value, 0.0f, 0.95f);
        } else if (param == "mix") {
            mix_ = std::clamp(value, 0.0f, 1.0f);
        } else if (param == "base_delay_ms") {
            baseDelaySec_ = std::clamp(value, 0.1f, 5.0f) * 0.001f; // ms→s
            const size_t baseSmp  = secToSamples(baseDelaySec_);
            const size_t depthSmp = secToSamples(depthSec_);
            const size_t total    = std::max<size_t>(baseSmp + depthSmp + 8, 512);
            if (dBuffer_.size() != total) {
                dBuffer_.assign(total, 0.0f);
                writePos_ = 0;
            }
        }
    }
}